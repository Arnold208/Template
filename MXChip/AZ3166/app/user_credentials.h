#ifndef USER_CREDENTIALS_H
#define USER_CREDENTIALS_H

// --- Azure Event Grid Certificates (DER Format) ---
// IMPORTANT: NetX Secure TLS requires certificates in DER (binary) format.
// You can convert PEM to DER using openssl:
// openssl x509 -in cert.pem -out cert.der -outform DER
// Then convert the binary file to a C hex array (e.g., using 'xxd -i cert.der').

// 1. Device Certificate (Client Certificate)
// This must match the Client ID/Username in app_config.h
const unsigned char device_cert_der[] = {
    0x00 // PLACEHOLDER: Paste your device certificate DER bytes here (e.g., 0x30, 0x82, ...)
};
const unsigned int device_cert_len = 1; // UPDATE THIS LENGTH

// 2. Device Private Key
// Must correspond to the device certificate.
const unsigned char device_key_der[] = {
    0x00 // PLACEHOLDER: Paste your device private key DER bytes here
};
const unsigned int device_key_len = 1; // UPDATE THIS LENGTH

// 3. Trusted Root CA (DigiCert Global Root G3)
// Required to verify the Event Grid server.
// Link: https://cacerts.digicert.com/DigiCertGlobalRootG3.crt.pem
const unsigned char root_ca_der[] = {
    0x00 // PLACEHOLDER: Paste the Root CA DER bytes here
};
const unsigned int root_ca_len = 1; // UPDATE THIS LENGTH

#endif // USER_CREDENTIALS_H
