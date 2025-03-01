#include "client.h"

#include <iostream>
#include <memory>
#include <memory>
#include <memory>

#include <asio/connect.hpp>
#include <asio/detached.hpp>
#include <asio/read_until.hpp>

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"  // for Ref

Client::Client(std::string_view host, std::string_view port) {
    connect(host, port);
}

Client::~Client() {
    io_context_thread_.join();
}

void Client::startReceiver() {
    co_spawn(socket_.get_executor(),
             [self = shared_from_this()] { return self->receiver(); },
             asio::detached);
}

std::string Client::getNickname() {
    auto screen = ftxui::ScreenInteractive::TerminalOutput();

    std::string name;
    ftxui::Component input_name = ftxui::Input(&name);
    input_name |= ftxui::CatchEvent([&](ftxui::Event event) {
        if (event == ftxui::Event::Character('\n')) {
            screen.Exit();
            return true;
        }
        return false;
    });

    auto component = ftxui::Container::Vertical({
        input_name
    });

    auto renderer = Renderer(component, [&] {
        return ftxui::vbox(ftxui::text("Enter your nickname:"),
                           ftxui::separator(),
                           input_name->Render()
               ) | ftxui::border;
    });

    screen.Loop(renderer);

    return name;
}

void Client::joinChat() {
    nlohmann::json payload;
    payload["type"] = "join";
    payload["nickname"] = getNickname();
    sendPayload(payload.dump());
}

void Client::setScreenToChat() {
    ftxui::Component input_message = ftxui::Input(&message_text_, "Message");
    input_message |= ftxui::CatchEvent([&](ftxui::Event event) {
        if (event == ftxui::Event::Character('\n')) {
            sendMessage();
            message_text_.clear();
            return true;
        }
        return false;
    });

    auto component = ftxui::Container::Vertical({
        input_message
    });

    auto renderer = ftxui::Renderer(component, [input_message, client = shared_from_this()]() {
        ftxui::Elements elements;
        for (const auto& message: client->messages_container_) {
            elements.push_back(ftxui::paragraph(message));
        }
        return ftxui::vbox(
            ftxui::vbox(elements) | ftxui::border | ftxui::flex,
            ftxui::vbox(input_message->Render()) | ftxui::border
        );
    });

    ui_ = renderer;
}

void Client::start() {
    io_context_thread_ = std::thread([&]() {
        io_context_.run();
    });

    startReceiver();
    joinChat();

    setScreenToChat();
    screen_.Loop(ui_);
}

void Client::connect(const std::string_view host, const std::string_view port) {
    try {
        asio::ip::tcp::resolver resolver(io_context_);
        asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(host, port);
        asio::connect(socket_, endpoint);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Client::stop() {
    socket_.close();
    work_guard_.reset();
    io_context_.stop();
    screen_.Exit();
}

void Client::sendPayload(const std::string& payload) {
    socket_.write_some(asio::buffer(payload + "\n"));
}

void Client::sendMessage() {
    nlohmann::json payload;
    payload["type"] = "message";
    payload["message"] = message_text_;
    sendPayload(payload.dump());
}

void Client::processMessage(const std::string_view message) {
    messages_container_.emplace_back(message);
    screen_.PostEvent(ftxui::Event::Custom);
}

asio::awaitable<void> Client::receiver() {
    try {
        for (std::string data;;) {
            co_await async_read_until(socket_,
                                      asio::dynamic_buffer(data, 1024), "\n",
                                      asio::use_awaitable);
            processMessage(data);
            data.erase();
        }
    } catch (std::exception&) {
        stop();
    }
}
