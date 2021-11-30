#include <nlohmann/json.hpp>

#include "senzor_server.h"

namespace rempos {

namespace senzor_server {

using json = nlohmann::json;

namespace json_util {

[[nodiscard]] inline SenzorResult ReadSenzorData(const json& input) {
    const auto& accelerStruct = input["accelerStruct"];
    const auto& gpsStruct = input["gpsStruct"];
    const auto& gyroStruct = input["gyroStruct"];
    const auto& pressStruct = input["pressStruct"];
    const auto& timestruct = input["timestruct"];
    const auto& userAccelerStruct = input["userAccelerStruct"];

    return {
        {
            accelerStruct["x"].get<float>(),
            accelerStruct["y"].get<float>(),
            accelerStruct["z"].get<float>()
        },
        {
            userAccelerStruct["ux"].get<float>(),
            userAccelerStruct["uy"].get<float>(),
            userAccelerStruct["uz"].get<float>()
        },
        // Adapt for Source Engine
        {
            -gyroStruct["pitch"].get<float>(),
            gyroStruct["yaw"].get<float>(),
            gyroStruct["roll"].get<float>()
        },
        {
            gpsStruct["latitude"].get<double>(),
            gpsStruct["longitude"].get<double>()
        },
        pressStruct["pressure"].get<double>(),
        timestruct["timeMark"].get<double>()
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
    {
        json payload_json = json::parse(msg->get_payload());
        
        assert(callback_);
        callback_(std::move(json_util::ReadSenzorData(payload_json)));
    }

    // Acknowledgement
    server_.send(hdl, nullptr, 0, websocketpp::frame::opcode::binary);
}

} // namespace websocket

} // namespace rempos