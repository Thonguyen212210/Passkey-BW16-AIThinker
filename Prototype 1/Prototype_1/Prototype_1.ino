#include <Arduino.h>
#include "fido_ble.h"
#include "crypto_mbedtls.h"
#include "storage.h"

// In AmebaD (BW16), the BOOT button is usually on PA27.
// If this pin is incorrect for your specific BW16, please change it.
#define BOOT_BUTTON_PIN PA27 

bool user_presence_confirmed = false;
static uint32_t last_button_press = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Starting BW16 FIDO2 BLE Passkey...");

    // Initialize the BOOT button as input with pullup.
    // The button typically pulls the pin to GND when pressed.
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

    // Initialize modules
    storage_init();
    crypto_init();
    fido_ble_init();
    
    Serial.println("Initialization complete. Waiting for BLE connection...");
}

void loop() {
    // Read the BOOT button state. (LOW means pressed if it's pull-to-GND).
    bool button_pressed = (digitalRead(BOOT_BUTTON_PIN) == LOW);
    
    if (button_pressed) {
        if (millis() - last_button_press > 500) { // Simple debounce
            user_presence_confirmed = true;
            Serial.println("User Presence (UP) Confirmed! Ready for WebAuthn request.");
            last_button_press = millis();
        }
    }
    
    // In a real application, you might want to flash an LED to indicate 
    // that a FIDO request is pending and waiting for User Presence.
    // For this PoC, we just continually check the button and set the flag.
    
    delay(50);
}
