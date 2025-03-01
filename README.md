# C++ chat client & server

![chat](https://github.com/user-attachments/assets/10a1af7c-09f8-492d-a366-04559473cad9)

Simple chat server and client on C++ with [Asio](https://think-async.com/Asio/), [FTXUI](https://github.com/ArthurSonzogni/FTXUI) and [JSON](https://github.com/nlohmann/json) libraries.

## Requirements

- [Asio C++ Library](https://think-async.com/Asio/) for network capabilites;
- [JSON for Modern C++](https://github.com/nlohmann/json) library for data serialization between client and server;
- [C++ Functional Terminal User Interface](https://github.com/ArthurSonzogni/FTXUI) library for client interface.

## Usage
### Server

Specify port in the arguments on which the server will accept connections: `.\server.exe 4200`.

### Client

Specify the server host and port: `.\client.exe 127.0.0.1 4200`.
