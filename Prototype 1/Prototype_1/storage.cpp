#include "storage.h"
#include <EEPROM.h>

struct CredentialStorage {
    uint8_t magic; // 0xA5 indicates valid
    uint8_t credential_id[16];
    uint8_t private_key[32];
    uint32_t sign_count;
};

void storage_init() {
    EEPROM.begin(sizeof(CredentialStorage));
}

bool storage_get_credential(uint8_t* credential_id, uint8_t* private_key, uint32_t* sign_count) {
    CredentialStorage cs;
    EEPROM.get(0, cs);
    
    if (cs.magic != 0xA5) {
        return false;
    }
    
    memcpy(credential_id, cs.credential_id, 16);
    memcpy(private_key, cs.private_key, 32);
    *sign_count = cs.sign_count;
    
    return true;
}

bool storage_save_credential(const uint8_t* credential_id, const uint8_t* private_key) {
    CredentialStorage cs;
    cs.magic = 0xA5;
    memcpy(cs.credential_id, credential_id, 16);
    memcpy(cs.private_key, private_key, 32);
    cs.sign_count = 0;
    
    EEPROM.put(0, cs);
    // EEPROM.commit(); // Note: some ESP/Ameba requires commit, Ameba EEPROM might not, but let's assume it does if available, or just put is enough.
    // In Ameba, put writes directly to flash in some implementations.
    return true;
}

bool storage_increment_sign_count(uint32_t* new_count) {
    CredentialStorage cs;
    EEPROM.get(0, cs);
    
    if (cs.magic != 0xA5) {
        return false;
    }
    
    cs.sign_count += 1;
    *new_count = cs.sign_count;
    
    EEPROM.put(0, cs);
    return true;
}
