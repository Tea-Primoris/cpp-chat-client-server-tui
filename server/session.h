#pragma once;

#include <deque>
#include <memory>
#include <string>

#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/steady_timer.hpp>

class Core;

using asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, Core& core);

    void start();

    void deliver(const std::string& msg);

private:
    void processMessage(std::string msg);

    asio::awaitable<void> reader();

    asio::awaitable<void> writer();

    void stop();

    tcp::socket socket_;
    asio::steady_timer timer_;

    Core& core_;
    std::string nickname_;
    std::deque<std::string> write_msgs_;
};
