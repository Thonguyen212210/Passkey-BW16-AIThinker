#ifndef CRYPTO_MBEDTLS_H
#define CRYPTO_MBEDTLS_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/sha256.h"
#include "mbedtls/error.h"
#ifdef __cplusplus
}
#endif

void crypto_init();
void crypto_free();
bool crypto_generate_keypair(uint8_t* private_key, uint8_t* public_key_x, uint8_t* public_key_y);
bool crypto_sign(const uint8_t* private_key, const uint8_t* hash, uint8_t* sig_r, uint8_t* sig_s);
void crypto_sha256(const uint8_t* data, size_t len, uint8_t* hash);

#endif // CRYPTO_MBEDTLS_H
