#ifndef CTAP2_H
#define CTAP2_H

#include <Arduino.h>

// CTAP Command codes
#define CTAP_CMD_MAKE_CREDENTIAL 0x01
#define CTAP_CMD_GET_ASSERTION   0x02
#define CTAP_CMD_GET_INFO        0x04
#define CTAP_CMD_CLIENT_PIN      0x06
#define CTAP_CMD_RESET           0x07

// CTAP Status codes
#define CTAP1_ERR_SUCCESS          0x00
#define CTAP2_ERR_INVALID_COMMAND  0x01
#define CTAP2_ERR_UP_REQUIRED      0x02
#define CTAP2_ERR_UNSUPPORTED_ALGORITHM 0x26
#define CTAP2_ERR_ACTION_TIMEOUT   0x3A

void ctap2_handle_request(const uint8_t* req, size_t len);

#endif // CTAP2_H
