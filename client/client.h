#pragma once
#include <thread>
#include <asio/co_spawn.hpp>
#include <asio/ip/tcp.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <nlohmann/json.hpp>

class Client : public std::enable_shared_from_this<Client> {

public:
    Client() = delete;

    Client(std::string_view host, std::string_view port);

    ~Client();

    void start();

private:
    asio::io_context io_context_;
    asio::executor_work_guard<asio::io_context::executor_type> work_guard_ = make_work_guard(io_context_);
    asio::ip::tcp::socket socket_ = asio::ip::tcp::socket(io_context_);
    std::thread io_context_thread_;

    std::vector<std::string> messages_container_;
    std::string message_text_;

    ftxui::Component ui_;
    ftxui::ScreenInteractive screen_ = ftxui::ScreenInteractive::Fullscreen();


    void connect(const std::string_view host, const std::string_view port);

    void stop();

    void sendPayload(const std::string& payload);

    void sendMessage();

    void processMessage(const std::string_view message);

    void startReceiver();

    std::string getNickname();

    void joinChat();

    void setScreenToChat();

    asio::awaitable<void> receiver();
};
