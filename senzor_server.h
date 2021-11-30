#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <GarrysMod/Lua/SourceCompat.h>

namespace rempos {

namespace senzor_server {

struct SenzorResult {
    Vector acceleration;
    Vector user_acceleration;
    QAngle gyroscope;
    struct Coordinates {
        double lat = 0.0, lng = 0.0;
    } gps;
    double pressure = 0.0;
    double timecode = 0.0;
};

class Server {
public:
    using WebSocketServer = websocketpp::server<websocketpp::config::asio>;
    using CallbackType = std::function<void(SenzorResult&&)>;

public:
    Server(CallbackType callback);

    Server(Server&) = delete;

    Server(Server&&) = delete;

    ~Server() {
        Stop();
    }

public:

    Server& operator=(Server&) = delete;

    Server& operator=(Server&&) = delete;

public:
    inline void Run(std::string ip, std::string port) {
        server_.listen(ip, port);

        server_.start_accept();

        server_.run();
    }

    inline void Run(uint16_t port) {
        server_.listen(port);

        server_.start_accept();

        server_.run();
    }

    inline void Stop() {
        if (server_.is_listening()) {
            server_.stop_listening();
        }

        websocketpp::lib::error_code code;
        if (server_.get_con_from_hdl(last_connection_, code)) {
            server_.close(last_connection_, websocketpp::close::status::going_away, "Server stopped");
        }

        // Is it safe to stop server from other thread?
        server_.stop();
    }

private:
    void OnOpen(websocketpp::connection_hdl hdl);

    void OnMessage(websocketpp::connection_hdl hdl, WebSocketServer::message_ptr msg);

private:
    WebSocketServer server_;
    websocketpp::connection_hdl last_connection_;
    const CallbackType callback_;
};

} // namespace websocket

} // namespace rempos