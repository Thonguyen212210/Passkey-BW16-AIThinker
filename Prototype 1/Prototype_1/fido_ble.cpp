#include "fido_ble.h"
#include "ctap2.h"
#include <Arduino.h>

BLECharacteristic* pFidoStatus = nullptr;
bool deviceConnected = false;

// Buffer for reassembling BLE FIDO fragments
static uint8_t rx_buffer[1024];
static size_t rx_offset = 0;
static size_t rx_expected_len = 0;
static uint8_t current_cmd = 0;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    }
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        pServer->startAdvertising(); // restart advertising
    }
};

class FidoControlPointCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            fido_ble_process_packet((const uint8_t*)rxValue.data(), rxValue.length());
        }
    }
};

void fido_ble_init() {
    BLEDevice::init("BW16-FIDO2-Key");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pFidoService = pServer->createService(FIDO_SERVICE_UUID);

    // Control Point Length (Max fragment size)
    BLECharacteristic *pLenChar = pFidoService->createCharacteristic(
                                    FIDO_CONTROL_POINT_LEN_UUID,
                                    BLECharacteristic::PROPERTY_READ
                                );
    uint8_t cpLen[] = {0x02, 0x00}; // 512 bytes max (Big Endian)
    pLenChar->setValue(cpLen, 2);

    // Service Revision
    BLECharacteristic *pRevChar = pFidoService->createCharacteristic(
                                    FIDO_SERVICE_REVISION_UUID,
                                    BLECharacteristic::PROPERTY_READ
                                );
    uint8_t srvRev[] = {0x20}; // U2F 1.2 or FIDO2 (0x20)
    pRevChar->setValue(srvRev, 1);

    // Control Point (Write)
    BLECharacteristic *pCpChar = pFidoService->createCharacteristic(
                                    FIDO_CONTROL_POINT_UUID,
                                    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
                                );
    pCpChar->setCallbacks(new FidoControlPointCallbacks());

    // Status (Notify)
    pFidoStatus = pFidoService->createCharacteristic(
                                    FIDO_STATUS_UUID,
                                    BLECharacteristic::PROPERTY_NOTIFY
                                );

    pFidoService->start();

    // Advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(FIDO_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
}

void fido_ble_send_fragment(uint8_t cmd, const uint8_t* data, size_t total_len, size_t offset) {
    if (!pFidoStatus || !deviceConnected) return;

    uint8_t packet[64];
    size_t packet_len = 0;
    
    if (offset == 0) {
        // INIT fragment
        packet[0] = cmd;
        packet[1] = (total_len >> 8) & 0xFF;
        packet[2] = total_len & 0xFF;
        size_t chunk = min(total_len, (size_t)61);
        memcpy(&packet[3], data, chunk);
        packet_len = chunk + 3;
    } else {
        // CONT fragment
        uint8_t seq = (offset - 61) / 63;
        packet[0] = seq;
        size_t chunk = min(total_len - offset, (size_t)63);
        memcpy(&packet[1], data + offset, chunk);
        packet_len = chunk + 1;
    }

    pFidoStatus->setValue(packet, packet_len);
    pFidoStatus->notify();
    delay(10); // tiny delay between packets
}

void fido_ble_send_msg(const uint8_t* data, size_t len) {
    size_t offset = 0;
    while (offset < len) {
        fido_ble_send_fragment(FIDO_CMD_MSG, data, len, offset);
        if (offset == 0) {
            offset += 61;
        } else {
            offset += 63;
        }
    }
}

void fido_ble_send_error(uint8_t error_code) {
    uint8_t data[] = { error_code };
    fido_ble_send_fragment(FIDO_CMD_ERROR, data, 1, 0);
}

void fido_ble_process_packet(const uint8_t* packet, size_t length) {
    if (length == 0) return;

    if (packet[0] >= 0x80) {
        // INIT packet
        current_cmd = packet[0];
        rx_expected_len = (packet[1] << 8) | packet[2];
        size_t chunk = length - 3;
        if (chunk > rx_expected_len) chunk = rx_expected_len;
        
        memcpy(rx_buffer, &packet[3], chunk);
        rx_offset = chunk;
    } else {
        // CONT packet
        // packet[0] is sequence number
        size_t chunk = length - 1;
        if (rx_offset + chunk > rx_expected_len) {
            chunk = rx_expected_len - rx_offset;
        }
        memcpy(&rx_buffer[rx_offset], &packet[1], chunk);
        rx_offset += chunk;
    }

    if (rx_offset >= rx_expected_len) {
        // Full message received
        if (current_cmd == FIDO_CMD_MSG) {
            ctap2_handle_request(rx_buffer, rx_expected_len);
        } else if (current_cmd == FIDO_CMD_PING) {
            // Echo ping
            size_t offset = 0;
            while (offset < rx_expected_len) {
                fido_ble_send_fragment(FIDO_CMD_PING, rx_buffer, rx_expected_len, offset);
                if (offset == 0) offset += 61;
                else offset += 63;
            }
        } else if (current_cmd == FIDO_CMD_CANCEL) {
            // Cancel current operation
            fido_ble_send_error(0x05); // ERROR_TIMEOUT / CANCEL
        } else {
            fido_ble_send_error(0x01); // INVALID_CMD
        }
        
        rx_offset = 0;
        rx_expected_len = 0;
    }
}
