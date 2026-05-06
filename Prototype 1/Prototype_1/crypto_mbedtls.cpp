#include "crypto_mbedtls.h"

static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static bool crypto_initialized = false;

void crypto_init() {
    if (crypto_initialized) return;
    
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    
    const char *pers = "fido2_ble_passkey";
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                          (const unsigned char *) pers, strlen(pers));
                          
    crypto_initialized = true;
}

void crypto_free() {
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    crypto_initialized = false;
}

bool crypto_generate_keypair(uint8_t* private_key, uint8_t* public_key_x, uint8_t* public_key_y) {
    if (!crypto_initialized) return false;
    
    mbedtls_ecdsa_context ecdsa;
    mbedtls_ecdsa_init(&ecdsa);
    
    int ret = mbedtls_ecdsa_genkey(&ecdsa, MBEDTLS_ECP_DP_SECP256R1, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        mbedtls_ecdsa_free(&ecdsa);
        return false;
    }
    
    // Extract private key
    mbedtls_mpi_write_binary(&ecdsa.d, private_key, 32);
    
    // Extract public key (X and Y coordinates)
    mbedtls_mpi_write_binary(&ecdsa.Q.X, public_key_x, 32);
    mbedtls_mpi_write_binary(&ecdsa.Q.Y, public_key_y, 32);
    
    mbedtls_ecdsa_free(&ecdsa);
    return true;
}

bool crypto_sign(const uint8_t* private_key, const uint8_t* hash, uint8_t* sig_r, uint8_t* sig_s) {
    if (!crypto_initialized) return false;
    
    mbedtls_ecdsa_context ecdsa;
    mbedtls_ecdsa_init(&ecdsa);
    
    mbedtls_ecp_group_load(&ecdsa.grp, MBEDTLS_ECP_DP_SECP256R1);
    mbedtls_mpi_read_binary(&ecdsa.d, private_key, 32);
    
    mbedtls_mpi r, s;
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);
    
    int ret = mbedtls_ecdsa_sign(&ecdsa.grp, &r, &s, &ecdsa.d, hash, 32, mbedtls_ctr_drbg_random, &ctr_drbg);
    
    if (ret == 0) {
        mbedtls_mpi_write_binary(&r, sig_r, 32);
        mbedtls_mpi_write_binary(&s, sig_s, 32);
    }
    
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);
    mbedtls_ecdsa_free(&ecdsa);
    
    return (ret == 0);
}

void crypto_sha256(const uint8_t* data, size_t len, uint8_t* hash) {
    mbedtls_sha256_context sha_ctx;
    mbedtls_sha256_init(&sha_ctx);
    mbedtls_sha256_starts(&sha_ctx, 0); // 0 = SHA-256
    mbedtls_sha256_update(&sha_ctx, data, len);
    mbedtls_sha256_finish(&sha_ctx, hash);
    mbedtls_sha256_free(&sha_ctx);
}
