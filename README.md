# gmsv_rempos
Garry's Mod C++ module that connects to iOS app Senzor by Stanislav Révay via websockets and provides acceleration, user acceleration, gyroscope, gps and pressure data from the device.

Built with [websocketpp](https://github.com/zaphoyd/websocketpp) library.

## Demo

https://user-images.githubusercontent.com/5685050/143723137-593afe00-7c64-4690-87ab-0fe94f25c7de.mp4

## Using
Check [releases page](https://github.com/GitSparTV/gmsv_rempos/releases) for pre-built binaries.

Put `.dll` into garrysmod/lua/bin/ folder.

## Building
Requirements: CMake, C++17, [Websocketpp](https://github.com/zaphoyd/websocketpp), [Standalone ASIO](https://think-async.com/Asio/AsioStandalone.html) or Boost.ASIO, [nlohmann's JSON](https://github.com/nlohmann/json), [gmod-module-base]().

```sh
mkdir build
cd build
cmake ../ -DGMOD_MODULE_BASE="<path_to_gmod_module_base_root>" -DWEBSOCKETPP="<path_to_websockepp_root>" -DASIO="<path_to_asio>" -DNLOHMANN_JSON="<path_to_nlohmann_json_root>" -DCMAKE_BUILD_TYPE=Release -A Win32
cmake --build . --config=Release --verbose
```

To build with Boost.ASIO add `-DASIO_STANDALONE=OFF`.

`-A Win32` is requires only for x86 (x32) builds.

Quick note: gmod-module-base requires small patch in the SourceCompat.h:
```c++
        Vector(float x, float y, float z)
            : x(x)
            , y(y)
            , z(z)
        {}
```

## Documentation

First of all call `require("rempos")`, this will populate `RemPos` global table.

### `boolean = RemPos.Initialize()`
Initializes websocket server on `0.0.0.0` with default port defined in gmsv_rempos.cpp (8080).

Must be called before using other functions.

Returns true on success, otherwise raises error.

### `boolean = RemPos.Initialize(number port)`
Initializes websocket server on `0.0.0.0` with specified `port`.

Must be called before using other functions.

Returns true on success, otherwise raises error.

### `boolean = RemPos.Initialize(string ip, string port)`
Initializes websocket server on `ip` with specified `port`.

Must be called before using other functions.

Returns true on success, otherwise raises error.

### `SenzorResult = RemPos.GetData()`
Returns all measures. See [SenzorResult]() struct for fields.

### `Vector = RemPos.GetAcceleration()`
Returns acceleration sensor value as Vector. (Sensor unit: m/s^2)

### `number, number = RemPos.GetGPS()`
Returns acceleration sensor value as 2 numbers: latitude and longitude (Sensor unit: degrees) 

### `Angle = RemPos.GetGyroscope()`
Returns gyroscope sensor value as Angle. (Sensor unit: degrees)

### `number = RemPos.GetPressure()`
Returns pressure sensor value as number. (Sensor unit: kPa)

### `number = RemPos.GetTimecode()`
Returns timecode of the current packer of measures as number.

Can be used to track time difference.

### `Vector = RemPos.GetUserAcceleration()`
Returns user acceleration sensor value as Vector. (Sensor unit: m/s^2)

### `SenzorResult`
Struct for all measures.
```lua
{
    acceleration = Vector,
    gps_lat = number,
    gps_lng = number,
    gyroscope = Angle,
    pressure = number,
    timecode = number,
    user_acceleration = Vector 
}
```

## Example:
```lua
require("rempos")

RemPos.Initialize()

local ent
local lastdata = RemPos.GetData()

hook.Add("PlayerSay", "RemPos", function(ply, text)
    ent = Entity(1):GetEyeTrace().Entity
end)

hook.Add("Think", "RemPos", function()
    local newdata = RemPos.GetData()
    if newdata.timecode == lastdata.timecode then return end

    if newdata.gyroscope ~= lastdata.gyroscope then
        ent:SetAngles(newdata.gyroscope)
    end

    lastdata = newdata
end)
```
