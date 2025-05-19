#include "chat_client.h"
#include "cryptography.h"
#include <iostream>
#include <cerrno>  // For errno
#include <cstring>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <vector>
#include <oqs/oqs.h>

void send_loop_client(int sock) {
    char buffer[1024];
    while (true) {
        std::cout << "You send: " << std::flush;
        std::cin.getline(buffer, sizeof(buffer));
        if (send(sock, buffer, strlen(buffer), 0) == -1) {
            std::cerr << "[Client] Send failed: " << strerror(errno) << "\n";
            break;
        }
    }
}

void receive_loop_client(int sock) {
    char buffer[1024];
    while (true) {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            std::cerr << "[Client] Server disconnected.\n";
            break;
        }
        buffer[bytes] = '\0';
        std::cout << "\nServer: " << buffer << "\nYou: " << std::flush;
    }
}

void start_client() {

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "[Client] Socket creation failed\n";
        return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9000);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "[Client] Connection failed: " << strerror(errno) << "\n";
        close(sock);
        return;
    }
    std::cout << "[Client] Connected to server!\n";

    OQS_KEM* kem = OQS_KEM_new(OQS_KEM_alg_kyber_512);
    if (!kem) {
        std::cerr << "[Client] Failed to create KEM\n";
        close(sock);
        return;
    }

    std::vector<uint8_t> public_key(kem->length_public_key);
    int received = recv(sock, public_key.data(), public_key.size(), 0);
    if (received != static_cast<int>(public_key.size())) {
        std::cerr << "[Client] Failed to receive public key\n";
        OQS_KEM_free(kem);
        close(sock);
        return;
    }

    std::vector<uint8_t> ciphertext(kem->length_ciphertext);
    std::vector<uint8_t> shared_secret(kem->length_shared_secret);
    if (OQS_KEM_encaps(kem, ciphertext.data(), shared_secret.data(), public_key.data()) != OQS_SUCCESS) {
        std::cerr << "[Client] Encapsulation failed\n";
        OQS_KEM_free(kem);
        close(sock);
        return;
    }

    if (send(sock, ciphertext.data(), ciphertext.size(), 0) != static_cast<ssize_t>(ciphertext.size())) {
        std::cerr << "[Client] Failed to send ciphertext\n";
        OQS_KEM_free(kem);
        close(sock);
        return;
    }

    OQS_KEM_free(kem);
    std::cout << "[Client] Secure channel established.\n";

    // Messaging
    std::thread sender(send_loop_client, sock);
    std::thread receiver(receive_loop_client, sock);

    sender.join();
    receiver.join();

    close(sock);
}