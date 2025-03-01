#include <iostream>

#include <asio/awaitable.hpp>
#include <asio/detached.hpp>
#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/signal_set.hpp>
#include <asio/use_awaitable.hpp>

#include "core.h"
#include "session.h"

using asio::ip::tcp;

asio::awaitable<void> create_listener(tcp::acceptor acceptor) {
    for (;;) {
        std::make_shared<Session>(co_await acceptor.async_accept(asio::use_awaitable), GetCore())->start();
    }
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: chat_server <port> [<port> ...]\n";
            return 1;
        }

        std::cout << "Starting server..." << std::endl;

        asio::io_context io_context(1);

        for (int i = 1; i < argc; ++i) {
            unsigned short port = std::atoi(argv[i]);
            co_spawn(io_context,
                     create_listener(tcp::acceptor(io_context, {tcp::v4(), port})),
                     asio::detached);
            std::cout << "Created listener at port " << port << std::endl;
        }

        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io_context.stop(); });

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
