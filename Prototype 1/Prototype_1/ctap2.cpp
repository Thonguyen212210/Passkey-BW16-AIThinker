#include "ctap2.h"
#include "fido_ble.h"
#include "cbor.h"
#include "crypto_mbedtls.h"
#include "storage.h"

extern bool user_presence_confirmed;

static void send_ctap_error(uint8_t err) {
    uint8_t buf[1] = { err };
    fido_ble_send_msg(buf, 1);
}

static void handle_get_info() {
    uint8_t resp_buf[256];
    resp_buf[0] = CTAP1_ERR_SUCCESS;
    
    CborEncoder encoder, mapEncoder, arrayEncoder;
    cbor_encoder_init(&encoder, resp_buf + 1, sizeof(resp_buf) - 1, 0);
    
    // Map of GetInfo response
    cbor_encoder_create_map(&encoder, &mapEncoder, 3);
    
    // 1: versions
    cbor_encode_int(&mapEncoder, 1);
    cbor_encoder_create_array(&mapEncoder, &arrayEncoder, 1);
    cbor_encode_text_stringz(&arrayEncoder, "FIDO_2_0");
    cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
    
    // 3: aaguid (16 bytes)
    cbor_encode_int(&mapEncoder, 3);
    uint8_t aaguid[16] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10};
    cbor_encode_byte_string(&mapEncoder, aaguid, 16);
    
    // 4: options
    cbor_encode_int(&mapEncoder, 4);
    CborEncoder optMap;
    cbor_encoder_create_map(&mapEncoder, &optMap, 1);
    cbor_encode_text_stringz(&optMap, "up"); // User presence supported and requires UP
    cbor_encode_boolean(&optMap, true);
    cbor_encoder_close_container(&mapEncoder, &optMap);
    
    cbor_encoder_close_container(&encoder, &mapEncoder);
    
    size_t len = cbor_encoder_get_buffer_size(&encoder, resp_buf + 1);
    fido_ble_send_msg(resp_buf, len + 1);
}

static void handle_make_credential(const uint8_t* req, size_t len) {
    if (!user_presence_confirmed) {
        send_ctap_error(CTAP2_ERR_UP_REQUIRED);
        return;
    }
    
    // In a real implementation we would parse req with CborParser
    // and extract clientDataHash, rpEntity, userEntity, pubKeyCredParams
    
    // For PoC: Generate Keypair and save it
    uint8_t priv[32], pub_x[32], pub_y[32];
    if (!crypto_generate_keypair(priv, pub_x, pub_y)) {
        send_ctap_error(CTAP2_ERR_UNSUPPORTED_ALGORITHM);
        return;
    }
    
    uint8_t cred_id[16] = {0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00};
    storage_save_credential(cred_id, priv);
    
    // Construct AuthData and response
    // PoC: Just send a very basic dummy successful response to MakeCredential
    uint8_t resp_buf[256];
    resp_buf[0] = CTAP1_ERR_SUCCESS;
    
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, resp_buf + 1, sizeof(resp_buf) - 1, 0);
    cbor_encoder_create_map(&encoder, &mapEncoder, 2);
    
    // 1: fmt
    cbor_encode_int(&mapEncoder, 1);
    cbor_encode_text_stringz(&mapEncoder, "none");
    
    // 2: authData (dummy 37 bytes: 32 rpIdHash, 1 flags, 4 signCount)
    cbor_encode_int(&mapEncoder, 2);
    uint8_t auth_data[37] = {0};
    auth_data[32] = 0x41; // UP=1, AT=1
    cbor_encode_byte_string(&mapEncoder, auth_data, 37);
    
    cbor_encoder_close_container(&encoder, &mapEncoder);
    
    size_t out_len = cbor_encoder_get_buffer_size(&encoder, resp_buf + 1);
    fido_ble_send_msg(resp_buf, out_len + 1);
    
    // Consume UP
    user_presence_confirmed = false;
}

static void handle_get_assertion(const uint8_t* req, size_t len) {
    if (!user_presence_confirmed) {
        send_ctap_error(CTAP2_ERR_UP_REQUIRED);
        return;
    }
    
    uint8_t cred_id[16], priv[32];
    uint32_t sign_cnt;
    
    if (!storage_get_credential(cred_id, priv, &sign_cnt)) {
        send_ctap_error(0x2E); // No credentials
        return;
    }
    
    storage_increment_sign_count(&sign_cnt);
    
    // Generate signature
    uint8_t dummy_hash[32] = {0}; // Should be from clientDataHash
    uint8_t sig_r[32], sig_s[32];
    crypto_sign(priv, dummy_hash, sig_r, sig_s);
    
    // Send response
    uint8_t resp_buf[256];
    resp_buf[0] = CTAP1_ERR_SUCCESS;
    
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, resp_buf + 1, sizeof(resp_buf) - 1, 0);
    cbor_encoder_create_map(&encoder, &mapEncoder, 2);
    
    // 2: authData
    cbor_encode_int(&mapEncoder, 2);
    uint8_t auth_data[37] = {0};
    auth_data[32] = 0x01; // UP=1
    auth_data[33] = (sign_cnt >> 24) & 0xFF;
    auth_data[34] = (sign_cnt >> 16) & 0xFF;
    auth_data[35] = (sign_cnt >> 8) & 0xFF;
    auth_data[36] = sign_cnt & 0xFF;
    cbor_encode_byte_string(&mapEncoder, auth_data, 37);
    
    // 3: signature (DER encoded usually, but here just raw for PoC)
    cbor_encode_int(&mapEncoder, 3);
    uint8_t sig[64];
    memcpy(sig, sig_r, 32);
    memcpy(sig + 32, sig_s, 32);
    cbor_encode_byte_string(&mapEncoder, sig, 64);
    
    cbor_encoder_close_container(&encoder, &mapEncoder);
    
    size_t out_len = cbor_encoder_get_buffer_size(&encoder, resp_buf + 1);
    fido_ble_send_msg(resp_buf, out_len + 1);
    
    // Consume UP
    user_presence_confirmed = false;
}

void ctap2_handle_request(const uint8_t* req, size_t len) {
    if (len == 0) return;
    
    uint8_t cmd = req[0];
    
    switch(cmd) {
        case CTAP_CMD_GET_INFO:
            handle_get_info();
            break;
        case CTAP_CMD_MAKE_CREDENTIAL:
            handle_make_credential(req + 1, len - 1);
            break;
        case CTAP_CMD_GET_ASSERTION:
            handle_get_assertion(req + 1, len - 1);
            break;
        default:
            send_ctap_error(CTAP2_ERR_INVALID_COMMAND);
            break;
    }
}
