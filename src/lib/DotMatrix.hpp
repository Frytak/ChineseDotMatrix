#pragma once

#include "BluezDevice.hpp"
#include <chrono>
#include <cstdint>
#include <functional>
#include <sdbus-c++/Types.h>
#include <sdbus-c++/sdbus-c++.h>

constexpr auto UUID_WRITE_CHAR = "0000fa02-0000-1000-8000-00805f9b34fb";
constexpr auto UUID_READ_CHAR  = "00002902-0000-1000-8000-00805f9b34fb";

typedef struct {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
} Color;

class DotMatrix : public bluez::BluezDeviceGetInterface {
private:
    // D-Bus connection
    std::shared_ptr<sdbus::IConnection> conn;

    bluez::BluezDeviceProxy device;

public:
    DotMatrix(std::shared_ptr<sdbus::IConnection> conn, sdbus::ObjectPath device_path);
    DotMatrix(std::shared_ptr<sdbus::IConnection> conn, bluez::BluezDevice device);
    DotMatrix(bluez::BluezDeviceProxy device);

    // Scan for DotMatrix devices
    static std::vector<bluez::BluezDevice> scan(std::shared_ptr<sdbus::IConnection> conn, std::function<void (bluez::BluezDevice device)> callback = [](auto){}, std::chrono::milliseconds duration = std::chrono::seconds(5));

    std::string getObjectPath() const;
    std::string getAddress() const;
    bluez::AddressType getAddressType() const;
    std::optional<std::string> getName() const;
    std::string getAlias() const;
    std::optional<std::map<std::uint16_t, sdbus::Variant>> getManufacturerData() const;

    void connect();
    void disconnect();

    void setRotate180(bool rotated);

    void setDrawingMode();

    /* Set a pixel to the specified color in drawing mode.
    *
    * Set pixel     : 0x0a, 0x00, 0x05, 0x01, 0x00
    * RGB color     : 0xff, 0x00, 0x00
    * Position(x,y) : 0x07, 0x11
    */
    void setPixel(std::uint8_t x, std::uint8_t y, Color color);
    void test();
};
