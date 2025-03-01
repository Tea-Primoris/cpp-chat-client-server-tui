#include "core.h"

#include "session.h"

void Core::addMember(const std::shared_ptr<Session>& session) {
    sessions_.push_back(session);
}

void Core::sendMessage(const std::string message) {
    for (auto it = sessions_.begin(); it != sessions_.end(); ++it) {
        std::weak_ptr<Session> session = *it;
        if (session.expired()) {
            it = sessions_.erase(it);
            continue;
        }
        session.lock()->deliver(message);
    }
}

Core& GetCore() {
    static Core core;
    return core;
}
