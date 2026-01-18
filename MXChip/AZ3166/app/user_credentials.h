#ifndef USER_CREDENTIALS_H
#define USER_CREDENTIALS_H

// --- Azure Event Grid Certificates (DER Format) ---

// 1. Device Certificate (Client Certificate) for "mxchip-test-eventgrid"
// This should be the DER (binary) version of your PEM certificate.
const unsigned char device_cert_der[] = {
    0x00 // PLACEHOLDER: Paste your device certificate DER bytes here
};
const unsigned int device_cert_len = 0; // PLACEHOLDER: Set the length of the array above

// 2. Device Private Key
// This should be the DER (binary) version of your private key.
const unsigned char device_key_der[] = {
    0x00 // PLACEHOLDER: Paste your device private key DER bytes here
};
const unsigned int device_key_len = 0; // PLACEHOLDER: Set the length of the array above

// 3. Trusted Root CA (DigiCert Global Root G3)
// Download from: https://cacerts.digicert.com/DigiCertGlobalRootG3.crt.pem
// Convert PEM to DER before pasting here.
const unsigned char root_ca_der[] = {
    0x00 // PLACEHOLDER: Paste the Root CA DER bytes here
};
const unsigned int root_ca_len = 0; // PLACEHOLDER: Set the length of the array above

#endif // USER_CREDENTIALS_H
