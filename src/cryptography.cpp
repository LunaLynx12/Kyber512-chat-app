#include <iostream>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <oqs/oqs.h>

std::string base64_encode(const uint8_t* data, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, data, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length - 1);  // Exclude the null terminator
    BIO_free_all(bio);
    return result;
}

std::string perform_kyber_key_exchange() {
    // Initialize the Kyber 512 KEM algorithm from liboqs
    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_kyber_512);

    if (kem == nullptr) {
        std::cerr << "Failed to initialize Kyber KEM!" << std::endl;
        return "";
    }

    // Generate keypair
    uint8_t *public_key = new uint8_t[kem->length_public_key];
    uint8_t *secret_key = new uint8_t[kem->length_secret_key];
    if (OQS_KEM_keypair(kem, public_key, secret_key) != OQS_SUCCESS) {
        std::cerr << "Failed to generate Kyber keypair!" << std::endl;
        delete[] public_key;
        delete[] secret_key;
        return "";
    }

    std::cout << "Kyber 512 Key Exchange completed!" << std::endl;

    // Convert public key to Base64 for readable output
    std::string base64_public_key = base64_encode(public_key, kem->length_public_key);

    // Clean up
    delete[] public_key;
    delete[] secret_key;
    OQS_KEM_free(kem);

    return base64_public_key;
}
