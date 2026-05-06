#ifndef FIDO_BLE_H
#define FIDO_BLE_H

#include <Arduino.h>
#include "BLEDevice.h"

// FIDO BLE Service UUID
#define FIDO_SERVICE_UUID "0000FFFD-0000-1000-8000-00805f9b34fb"

// FIDO BLE Characteristics
#define FIDO_CONTROL_POINT_UUID "F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB" // Write, Write No Response
#define FIDO_STATUS_UUID        "F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB" // Notify
#define FIDO_CONTROL_POINT_LEN_UUID "F1D0FFF3-DEAA-ECEE-B42F-C9BA7ED623BB" // Read
#define FIDO_SERVICE_REVISION_UUID "F1D0FFF4-DEAA-ECEE-B42F-C9BA7ED623BB" // Read

// FIDO U2F over BLE Framing Commands
#define FIDO_CMD_PING 0x81
#define FIDO_CMD_KEEPALIVE 0x82
#define FIDO_CMD_MSG 0x83
#define FIDO_CMD_CANCEL 0x85
#define FIDO_CMD_ERROR 0xBF

extern BLECharacteristic* pFidoStatus;
extern bool deviceConnected;

void fido_ble_init();
void fido_ble_send_msg(const uint8_t* data, size_t len);
void fido_ble_send_error(uint8_t error_code);
void fido_ble_process_packet(const uint8_t* packet, size_t length);

#endif // FIDO_BLE_H
