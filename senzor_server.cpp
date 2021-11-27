#include "senzor_server.h"
#include "json.h"

namespace rempos {

namespace senzor_server {

namespace util {

[[nodiscard]] inline SenzorResult ReadSenzorData(json::Dictionary map) {
    auto& accelerStruct = map.at("accelerStruct").AsMap();
    auto& gpsStruct = map.at("gpsStruct").AsMap();
    auto& gyroStruct = map.at("gyroStruct").AsMap();
    auto& pressStruct = map.at("pressStruct").AsMap();
    auto& timestruct = map.at("timestruct").AsMap();
    auto& userAccelerStruct = map.at("userAccelerStruct").AsMap();

    return {
        .accelerometer = {
            static_cast<float>(accelerStruct.at("x").AsDouble()),
            static_cast<float>(accelerStruct.at("y").AsDouble()),
            static_cast<float>(accelerStruct.at("z").AsDouble())
    },
        .user_accelerometer = {
            static_cast<float>(userAccelerStruct.at("ux").AsDouble()),
            static_cast<float>(userAccelerStruct.at("uy").AsDouble()),
            static_cast<float>(userAccelerStruct.at("uz").AsDouble())
    },
        // Adapt for Source Engine
        .gyroscope = {
            static_cast<float>(-gyroStruct.at("pitch").AsDouble()),
            static_cast<float>(gyroStruct.at("yaw").AsDouble()),
            static_cast<float>(gyroStruct.at("roll").AsDouble())
    },
        .gps = {
            gpsStruct.at("latitude").AsDouble(),
            gpsStruct.at("longitude").AsDouble()
    },
        .pressure = pressStruct.at("pressure").AsDouble(),
        .timecode = timestruct.at("timeMark").AsDouble()
    };
}

}

Server::Server(CallbackType callback) : callback_(callback) {
    // We don't need logging
    server_.set_access_channels(websocketpp::log::alevel::none);
    server_.set_error_channels(websocketpp::log::elevel::none);

    server_.init_asio();

    server_.set_open_handler(std::bind(
        &Server::OnOpen, this,
        std::placeholders::_1
    ));

    server_.set_message_handler(std::bind(
        &Server::OnMessage, this,
        std::placeholders::_1, std::placeholders::_2
    ));
}

void Server::OnOpen(websocketpp::connection_hdl hdl) {
    // First message is required by Senzor app
    server_.send(hdl, "Connected", websocketpp::frame::opcode::text);
    server_.send(hdl, "Connected to RemPos " REMPOS_VERSION, websocketpp::frame::opcode::text);
}

void Server::OnMessage(websocketpp::connection_hdl hdl, WebSocketServer::message_ptr msg) {
    // We will use streams for now
    {
        std::istringstream payload_stream(msg->get_payload());
        json::Dictionary data_json = json::Load(payload_stream).GetRoot().AsMap();

        assert(callback_);
        callback_(std::move(util::ReadSenzorData(data_json)));
    }

    // Acknowledgement
    server_.send(hdl, nullptr, 0, websocketpp::frame::opcode::binary);
}

} // namespace websocket

} // namespace rempos