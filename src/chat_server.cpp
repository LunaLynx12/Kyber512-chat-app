#include "chat_server.h"
#include "cryptography.h"
#include <iostream>
#include <cstring>
#include <thread>
#include <cerrno>  // For errno
#include <openssl/evp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <oqs/oqs.h>

void send_loop_server(int sock) {
    char buffer[1024];
    while (true) {
        std::cout << "You send: " << std::flush;
        std::cin.getline(buffer, sizeof(buffer));
        if (send(sock, buffer, strlen(buffer), 0) == -1) {
            std::cerr << "[Server] Send failed: " << strerror(errno) << "\n";
            break;
        }
    }
}

void receive_loop_server(int sock) {
    char buffer[1024];
    while (true) {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            std::cerr << "[Server] Client disconnected.\n";
            break;
        }
        buffer[bytes] = '\0';
        std::cout << "\nClient: " << buffer << "\nYou: " << std::flush;
    }
}

void start_server() {

    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[Server] Socket creation failed\n";
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9000);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[Server] Bind failed\n";
        close(server_fd);
        return;
    }

    if (listen(server_fd, 1) < 0) {
        std::cerr << "[Server] Listen failed\n";
        close(server_fd);
        return;
    }

    std::cout << "[Server] Waiting for connection on port 9000..." << std::endl;

    int client_socket = accept(server_fd, nullptr, nullptr);
    if (client_socket < 0) {
        std::cerr << "[Server] Accept failed\n";
        close(server_fd);
        return;
    }
    std::cout << "[Server] Client connected!" << std::endl;

    // Kyber Key Exchange
    OQS_KEM* kem = OQS_KEM_new(OQS_KEM_alg_kyber_512);
    if (!kem) {
        std::cerr << "[Server] Failed to create KEM\n";
        close(client_socket);
        close(server_fd);
        return;
    }

    std::vector<uint8_t> public_key(kem->length_public_key);
    std::vector<uint8_t> secret_key(kem->length_secret_key);
    if (OQS_KEM_keypair(kem, public_key.data(), secret_key.data()) != OQS_SUCCESS) {
        std::cerr << "[Server] Key pair generation failed\n";
        OQS_KEM_free(kem);
        close(client_socket);
        close(server_fd);
        return;
    }

    if (send(client_socket, public_key.data(), public_key.size(), 0) != static_cast<ssize_t>(public_key.size())) {
        std::cerr << "[Server] Failed to send public key\n";
        OQS_KEM_free(kem);
        close(client_socket);
        close(server_fd);
        return;
    }

    std::vector<uint8_t> ciphertext(kem->length_ciphertext);
    int received = recv(client_socket, ciphertext.data(), ciphertext.size(), 0);
    if (received != static_cast<int>(ciphertext.size())) {
        std::cerr << "[Server] Failed to receive ciphertext\n";
        OQS_KEM_free(kem);
        close(client_socket);
        close(server_fd);
        return;
    }

    std::vector<uint8_t> shared_secret(kem->length_shared_secret);
    if (OQS_KEM_decaps(kem, shared_secret.data(), ciphertext.data(), secret_key.data()) != OQS_SUCCESS) {
        std::cerr << "[Server] Decapsulation failed\n";
        OQS_KEM_free(kem);
        close(client_socket);
        close(server_fd);
        return;
    }

    OQS_KEM_free(kem);
    std::cout << "[Server] Secure channel established.\n";

    // Messaging
    std::thread sender(send_loop_server, client_socket);
    std::thread receiver(receive_loop_server, client_socket);

    sender.join();
    receiver.join();

    close(client_socket);
    close(server_fd);
}