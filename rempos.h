#pragma once

#include <thread>
#include <mutex>

#include "senzor_server.h"

namespace rempos {

class RemPos {
public:
    RemPos(uint16_t port)
        : server_(new senzor_server::Server(GenerateCallback())),
        worker_(new std::thread(&RemPos::RunServerByPort, this, port)) {}

    RemPos(std::string ip, std::string port)
        : server_(new senzor_server::Server(GenerateCallback())),
        worker_(new std::thread(&RemPos::RunServerByIPAndPort, this, std::move(ip), std::move(port))) {}

    ~RemPos() {
        server_->Stop();
        worker_->join();
    }

public:
    // Gets last data packet. I'm not sure if we should collect all packets in a queue to be read.
    [[nodiscard]] inline senzor_server::SenzorResult GetData() const {
        std::lock_guard lock(guard_);

        return last_data_;
    }

private:
    [[nodiscard]] inline senzor_server::Server::CallbackType GenerateCallback() {
        return [this](senzor_server::SenzorResult&& data) {
            this->UpdateData(std::move(data));
        };
    }

    void RunServerByPort(uint16_t port) {
        server_->Run(port);
    }

    void RunServerByIPAndPort(std::string ip, std::string port) {
        server_->Run(ip, port);
    }

    inline void UpdateData(senzor_server::SenzorResult&& data) {
        std::lock_guard lock(this->guard_);
        last_data_ = std::move(data);
    }

private:
    senzor_server::SenzorResult last_data_;
    mutable std::mutex guard_;
    std::unique_ptr<senzor_server::Server> server_;
    std::unique_ptr<std::thread> worker_;
};

} // namespace rempos