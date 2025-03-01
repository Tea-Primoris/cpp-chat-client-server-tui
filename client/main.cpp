#include <deque>
#include <iostream>
#include <memory>

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

#include "client.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: chat_client <host> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    auto client = std::make_shared<Client>(argv[1], argv[2]);
    client->start();

    return EXIT_SUCCESS;
}
