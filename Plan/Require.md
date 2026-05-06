# ROLE: 
Act as a Senior Embedded Security Engineer and Cryptography Expert, specializing in the FIDO2/CTAP2 protocols, BLE communication, and the Realtek Ameba IoT ecosystem.

# OBJECTIVE:
I want to build a Proof-of-Concept (PoC) FIDO2 WebAuthn Passkey (Security Key) using a **SparkletIoT BW16E board** (which houses the Realtek **RTL8720DN** MCU: ARM Cortex-M4, 512KB SRAM, Bluetooth 5.0 LE). 
The device will communicate with a host (laptop/smartphone) exclusively via **BLE (Bluetooth Low Energy)** to authenticate web logins.

# TECHNICAL CONSTRAINTS & REQUIREMENTS:
1. **Framework:** Use the **AmebaD Arduino SDK** (Realtek's official Arduino core).
2. **BLE Protocol:** Implement the official FIDO BLE Device Service (Service UUID: `0xFFFD`). Handle FIDO Control Point, FIDO Status, FIDO Control Point Length, and FIDO Service Revision characteristics.
3. **FIDO Protocol:** Implement CTAP2 (Client to Authenticator Protocol). Handle framing of APDUs over BLE.
4. **Cryptography:** The RTL8720DN does not have a dedicated Secure Element. We must implement cryptography in software using **mbedTLS** (which is native to the Ameba SDK) or a lightweight alternative like `micro-ecc`.
   - Must support ECDSA with the **secp256r1 (P-256)** curve for key generation and signing.
   - Must support **SHA-256** for hashing.
5. **Data Encoding:** Implement or integrate a lightweight **CBOR** (Concise Binary Object Representation) parser/encoder to handle CTAP2 authenticators' requests and responses.
6. **Storage:** Use the RTL8720DN's Flash memory (via `EEPROM` library or Flash API) to securely store the generated Private Keys and state counters (simulated secure storage).

# YOUR TASKS (Deliverables):
Please provide a comprehensive, step-by-step technical implementation plan. I do not need the entire codebase in one prompt, but I need the exact architecture and the critical C/C++ code components. Structure your response as follows:

### Phase 1: Architecture & Library Selection
- Define the exact libraries to use for CBOR (e.g., TinyCBOR, QCBOR) and Cryptography (mbedTLS API mapping for P-256).
- Explain the BLE GATT table structure required for the FIDO service on the Ameba SDK.

### Phase 2: Core Logic Implementation (Provide C/C++ Snippets)
- **Snippet 1 (BLE Init):** How to initialize the RTL8720DN BLE server with the `0xFFFD` FIDO service and required characteristics.
- **Snippet 2 (Crypto wrapper):** A function to generate a secp256r1 keypair and sign a SHA-256 hash using the Ameba mbedTLS implementation.
- **Snippet 3 (CTAP2 routing):** A high-level state machine or switch-case showing how incoming BLE GATT writes (MakeCredential, GetAssertion) are parsed and routed to the CBOR handler.

### Phase 3: Security Mitigation & Debugging
- Acknowledge the lack of a Secure Element and explain how to best obfuscate or protect the private key in the RTL8720DN's Flash memory.
- Provide instructions on how to test this BLE Passkey using `webauthn.io` or `chromium-fido-tool`.

**Important Note for the AI:** Do not give generic Arduino BLE examples. The code and logic must be strictly tailored to the capabilities and MbedTLS integration of the Realtek RTL8720DN / AmebaD architecture. Ensure CTAP2 over BLE framing rules are respected.