cmake_minimum_required(VERSION 3.30)
project(cpp_client_server_cli)

set(CMAKE_CXX_STANDARD 26)

find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ftxui CONFIG REQUIRED)

add_executable(server server/main.cpp
        server/session.cpp
        server/core.cpp)
target_link_libraries(server PRIVATE asio::asio nlohmann_json::nlohmann_json)

add_executable(client client/main.cpp
        client/client.cpp
        client/client.h)
target_link_libraries(client PRIVATE asio::asio nlohmann_json::nlohmann_json ftxui::dom ftxui::screen ftxui::component)