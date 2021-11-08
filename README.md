# gmsv_rempos
Use your iOS gyroscope and accelerometer data in Garry's Mod

Module uses evhttp (libevent) library and works with Sensor Logger (by Joe Crozier Software).

## `RemPos.Initialize(ip, port)`
Initializes http event listener. If arguments are not provided, they are default to `0.0.0.0`, `5555`.

## `vector = RemPos.GetAccelerometer()`
Returns latest accelerometer data as Vector. Returns `nil` if the value didn't changed or wasn't received yet.

## `vector = RemPos.GetGyroscope()`
Returns latest gyroscope data as Vector. Returns `nil` if the value didn't changed or wasn't received yet.

## `vector = RemPos.GetMagneticField()`
Returns latest magnetic field data as Vector. Returns `nil` if the value didn't changed or wasn't received yet.

## Example:
```lua
require("rempos")

RemPos.Initialize("192.168.1.5", 1234)
 
-- Let's say we have a prop at index 2
local test_entity = Entity(2)

hook.Add("Think", "RemPosTest", function()
    local accelerometer = RemPos.GetAccelerometer()
    local gyroscope = RemPos.GetGyroscope()
    local magnetic_field = RemPos.GetMagneticField()
    
    if accelerometer then
        test_entity:SetPos(test_entity:GetPos() + accelerometer)
    end
    
    if gyroscope then
        test_entity:SetAngles(test_entity:GetAngles() + gyroscope:Angle())
    end
    
    if magnetic_field then
        print(magnetic_field)
    end
end)
```
