#include "session.h"

#include <utility>
#include <iostream>

#include <asio/use_awaitable.hpp>
#include <asio/write.hpp>
#include <asio/read_until.hpp>
#include <asio/redirect_error.hpp>
#include <asio/detached.hpp>
#include <asio/co_spawn.hpp>

#include <nlohmann/json.hpp>

#include "core.h"


Session::Session(tcp::socket socket, Core& core): socket_(std::move(socket)),
                                                  timer_(socket_.get_executor()),
                                                  core_(core) {
    timer_.expires_at(std::chrono::steady_clock::time_point::max());
}

void Session::start() {
    co_spawn(socket_.get_executor(),
             [self = shared_from_this()] { return self->reader(); },
             asio::detached);

    co_spawn(socket_.get_executor(),
             [self = shared_from_this()] { return self->writer(); },
             asio::detached);
}

void Session::deliver(const std::string& msg) {
    write_msgs_.push_back(msg);
    timer_.cancel_one();
}

void Session::processMessage(std::string msg) {
    const nlohmann::json message = nlohmann::json::parse(msg);

    if (const std::string_view type = message["type"]; type == "join") {
        core_.addMember(shared_from_this());
        nickname_ = message.at("nickname").get<std::string>();
        core_.sendMessage("<Server> " + nickname_ + " joined\n");
        std::cout << "<Server> " + nickname_ + " joined" << std::endl;
    } else if (type == "message") {
        std::cout << nickname_ + ": " + message.at("message").get<std::string>() << std::endl;
        core_.sendMessage(nickname_ + ": " + message.at("message").get<std::string>() + "\n");
    }
}

asio::awaitable<void> Session::reader() {
    try {
        for (std::string read_msg;;) {
            std::size_t n = co_await async_read_until(socket_,
                                                      asio::dynamic_buffer(read_msg, 1024), "\n",
                                                      asio::use_awaitable);
            processMessage(read_msg);
            read_msg.erase();
        }
    } catch (std::exception&) {
        stop();
    }
}

asio::awaitable<void> Session::writer() {
    try {
        while (socket_.is_open()) {
            if (write_msgs_.empty()) {
                asio::error_code ec;
                co_await timer_.async_wait(redirect_error(asio::use_awaitable, ec));
            } else {
                co_await async_write(socket_,
                                     asio::buffer(write_msgs_.front()),
                                     asio::use_awaitable);
                write_msgs_.pop_front();
            }
        }
    } catch (std::exception&) {
        stop();
    }
}

void Session::stop() {
    if (!nickname_.empty()) {

        std::cout << "<Server> " + nickname_ + " left" << std::endl;
        core_.sendMessage("<Server> " + nickname_ + " left\n");
    }
    socket_.close();
    timer_.cancel();
}
