#include "rempos.h"

#include <GarrysMod/Lua/Interface.h>

namespace rempos {

namespace gmsv {
    static constexpr uint16_t kDefaultPort = 8080;
    static std::unique_ptr<RemPos> rempos_server = nullptr;

    void PushSenzorResult(GarrysMod::Lua::ILuaBase* LUA, senzor_server::SenzorResult& senzor) {
        LUA->CreateTable();

        LUA->PushVector(senzor.acceleration);
        LUA->SetField(-2, "acceleration");

        LUA->PushNumber(senzor.gps.lat);
        LUA->SetField(-2, "gps_lat");

        LUA->PushNumber(senzor.gps.lng);
        LUA->SetField(-2, "gps_lng");

        LUA->PushAngle(senzor.gyroscope);
        LUA->SetField(-2, "gyroscope");

        LUA->PushNumber(senzor.pressure);
        LUA->SetField(-2, "pressure");

        LUA->PushNumber(senzor.timecode);
        LUA->SetField(-2, "timecode");

        LUA->PushVector(senzor.user_acceleration);
        LUA->SetField(-2, "user_acceleration");

    }

    // 0 arguments: ip = localhost, port = kDefaultPort
    // 1 argument: ip = localhost, port = argument_1:number
    // 2 arguments: ip = argument_1:string, port = argument_2:string
    LUA_FUNCTION(Initialize) {
        if (rempos_server) {
            LUA->PushBool(true);

            return 1;
        }

        int top = LUA->Top();

        if (top <= 1) {
            uint16_t port = gmsv::kDefaultPort;

            if (top == 1) {
                port = static_cast<uint16_t>(LUA->GetNumber(1));
            }

            rempos_server.reset(new(std::nothrow) RemPos(port));
        } else if (top >= 2) {
            rempos_server.reset(new(std::nothrow) RemPos(LUA->CheckString(1), LUA->CheckString(2)));
        }

        if (!rempos_server) {
            LUA->ThrowError("Couldn't create RemPos object. Maybe not enough memory.");
        }

        LUA->PushBool(true);

        return 1;
    }

    // Get all sensors at once
    LUA_FUNCTION(GetData) {
        if (!rempos_server) {
            LUA->ThrowError("RemPos is not initialized");
        }

        senzor_server::SenzorResult data = rempos_server->GetData();

        PushSenzorResult(LUA, data);

        return 1;
    }

    // Single getters
    LUA_FUNCTION(GetAcceleration) {
        if (!rempos_server) {
            LUA->ThrowError("RemPos is not initialized");
        }

        senzor_server::SenzorResult data = rempos_server->GetData();

        LUA->PushVector(data.acceleration);

        return 1;
    }

    LUA_FUNCTION(GetGPS) {
        if (!rempos_server) {
            LUA->ThrowError("RemPos is not initialized");
        }

        senzor_server::SenzorResult data = rempos_server->GetData();

        LUA->PushNumber(data.gps.lat);
        LUA->PushNumber(data.gps.lng);

        return 2;
    }

    LUA_FUNCTION(GetGyroscope) {
        if (!rempos_server) {
            LUA->ThrowError("RemPos is not initialized");
        }

        senzor_server::SenzorResult data = rempos_server->GetData();

        LUA->PushAngle(data.gyroscope);

        return 1;
    }

    LUA_FUNCTION(GetPressure) {
        if (!rempos_server) {
            LUA->ThrowError("RemPos is not initialized");
        }

        senzor_server::SenzorResult data = rempos_server->GetData();

        LUA->PushNumber(data.pressure);

        return 1;
    }

    LUA_FUNCTION(GetTimecode) {
        if (!rempos_server) {
            LUA->ThrowError("RemPos is not initialized");
        }

        senzor_server::SenzorResult data = rempos_server->GetData();

        LUA->PushNumber(data.timecode);

        return 1;
    }

    LUA_FUNCTION(GetUserAcceleration) {
        if (!rempos_server) {
            LUA->ThrowError("RemPos is not initialized");
        }

        senzor_server::SenzorResult data = rempos_server->GetData();

        LUA->PushVector(data.user_acceleration);

        return 1;
    }
    
} // namespace gmsv

} // namespace rempos

/*
_G.RemPos = {
    Initialize: function,
    GetData: function,
    GetAcceleration: function,
    GetGPS: function,
    GetGyroscope: function,
    GetPressure: function,
    GetTimecode: function,
    GetUserAcceleration: function
}
*/
GMOD_MODULE_OPEN() {
    LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);

    LUA->CreateTable();

    LUA->PushCFunction(rempos::gmsv::Initialize);
    LUA->SetField(-2, "Initialize");

    LUA->PushCFunction(rempos::gmsv::GetData);
    LUA->SetField(-2, "GetData");

    LUA->PushCFunction(rempos::gmsv::GetAcceleration);
    LUA->SetField(-2, "GetAcceleration");

    LUA->PushCFunction(rempos::gmsv::GetGPS);
    LUA->SetField(-2, "GetGPS");

    LUA->PushCFunction(rempos::gmsv::GetGyroscope);
    LUA->SetField(-2, "GetGyroscope");

    LUA->PushCFunction(rempos::gmsv::GetPressure);
    LUA->SetField(-2, "GetPressure");

    LUA->PushCFunction(rempos::gmsv::GetTimecode);
    LUA->SetField(-2, "GetTimecode");

    LUA->PushCFunction(rempos::gmsv::GetUserAcceleration);
    LUA->SetField(-2, "GetUserAcceleration");

    LUA->SetField(-2, "RemPos");

    LUA->Pop();

    return 0;
}

GMOD_MODULE_CLOSE() {
    rempos::gmsv::rempos_server.reset(nullptr);

    return 0;
}