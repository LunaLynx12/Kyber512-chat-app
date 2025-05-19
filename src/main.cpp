#include <iostream>

#include "chat_server.h"
#include "chat_client.h"

int main() {
    std::string role;
    std::cout << "Select role (server/client): ";
    std::cin >> role;

    if (role == "server") {
        start_server();  // defined in chat_server.cpp
    } else if (role == "client") {
        start_client();  // defined in chat_client.cpp
    } else {
        std::cerr << "Invalid role selected." << std::endl;
        return 1;
    }

    return 0;
}

/*
#include "cryptography.h"
#include "lua_integration.h"

int main() {
    // Perform the Kyber key exchange
    std::string key = perform_kyber_key_exchange();
    std::cout << "Generated Public Key: " << key << std::endl;

    // Lua Script Example (Assume the script is a file or string)
    std::string script = "script.lua"; // Add the path to your Lua script
    execute_lua_script(script);

    return 0;
}
*/
