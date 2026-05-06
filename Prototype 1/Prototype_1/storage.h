#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

void storage_init();
bool storage_get_credential(uint8_t* credential_id, uint8_t* private_key, uint32_t* sign_count);
bool storage_save_credential(const uint8_t* credential_id, const uint8_t* private_key);
bool storage_increment_sign_count(uint32_t* new_count);

#endif // STORAGE_H
