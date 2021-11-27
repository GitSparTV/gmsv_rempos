# gmsv_rempos
Garry's Mod C++ module that connects to iOS app Senzor by Stanislav Révay via websockets and provides acceleration, user acceleration, gyroscope, gps and pressure data from the device.

Built with [websocketpp](https://github.com/zaphoyd/websocketpp) library.

## Demo

---

__[<img src="https://raw.githubusercontent.com/GitSparTV/GitSparTV/main/saythanks.svg" width="300">](https://gitspartv.github.io/GitSparTV/saythanks.html)__

---

## Using
Check [releases page](https://github.com/GitSparTV/gmsv_rempos/releases) for pre-built binaries.

Put `.dll` into garrysmod/lua/bin/ folder.

## Building
Requirements: CMake, C++20, Websocketpp headers, Standalone ASIO or Boost.ASIO, gmod-module-base.

```sh
mkdir build
cd build
cmake ../ -DGMOD_MODULE_BASE="<path_to_gmod_module_base_root" -DWEBSOCKETPP="path_to_websockepp_root" -DASIO="path_to_asio" -A Win32
cmake --build . --config=Release --verbose
```

To build with Boost.ASIO add `-DASIO_STANDALONE=OFF`.

## Documentation

First of all call `require("rempos")`, this will popular `RemPos` global table.

### `boolean = RemPos.Initialize()`
Initializes websocket server on localhost with default port defined in gmsv_rempos.cpp (8080).

Must be called before using other functions.

Returns true on success, otherwise raises error.

### `boolean = RemPos.Initialize(number port)`
Initializes websocket server on localhost with specified `port`.

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
