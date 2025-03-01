#pragma once
#include <memory>
#include <string>
#include <vector>

class Session;

class Core {
public:
    void addMember(const std::shared_ptr<Session>& session);

    void sendMessage(const std::string message);

private:
    std::vector<std::weak_ptr<Session>> sessions_;

    Core() = default;

    friend Core &GetCore();
};

Core &GetCore();
