# Passkey BW16 (AI-Thinker / Realtek RTL8720DN)

Proof-of-concept **FIDO2 / WebAuthn BLE authenticator** (passkey-style security key) for boards such as **BW16** (Realtek **RTL8720DN**) using the **AmebaD Arduino** core.

The firmware advertises the standard **FIDO BLE** service (`0xFFFD`), frames **CTAP2** over BLE, uses **mbedTLS** for P-256 operations, **TinyCBOR** (vendored) for CTAP2 encoding/decoding, and flash **storage** for credentials.

## Firmware location

Open this sketch in the Arduino IDE:

`Prototype 1/Prototype_1/Prototype_1.ino`

## Hardware notes

- Default **user presence** pin: `BOOT` on **PA27** (adjust in the sketch if your board differs).
- After flashing, press the button when a WebAuthn flow expects user presence.

## Requirements

- [Ameba Arduino](https://www.amebaiot.com/en/amebad-arduino-getting-started/) (AmebaD) package for your board.
- Project **TinyCBOR** sources are included under the sketch folder; cryptography targets the SDK’s **mbedTLS** integration.

## Documentation

See `Plan/Require.md` for the original architecture, constraints, and testing notes (e.g. webauthn.io / Chromium FIDO tooling).

## Status

This is a **research / PoC** build: some CTAP2 paths use simplified or placeholder parsing (see inline comments in `ctap2.cpp`). It is **not** a complete certified authenticator.

## License

Unless otherwise noted in individual third-party files (e.g. TinyCBOR), consider your own licensing when publishing derivative work.
