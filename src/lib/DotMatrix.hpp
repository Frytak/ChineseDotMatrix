#pragma once

#include "BluezDevice.hpp"
#include <chrono>
#include <functional>
#include <sdbus-c++/Types.h>
#include <sdbus-c++/sdbus-c++.h>

constexpr auto UUID_WRITE_CHAR = "0000fa02-0000-1000-8000-00805f9b34fb";
constexpr auto UUID_READ_CHAR  = "00002902-0000-1000-8000-00805f9b34fb";

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
    static std::vector<bluez::BluezDevice> scan(std::shared_ptr<sdbus::IConnection> conn, std::function<void (bluez::BluezDevice device)> callback = [](auto _){}, std::chrono::milliseconds duration = std::chrono::seconds(5));


    std::string getObjectPath() const;
    std::string getAddress() const;
    bluez::AddressType getAddressType() const;
    std::optional<std::string> getName() const;
    std::string getAlias() const;
    std::optional<std::map<std::uint16_t, sdbus::Variant>> getManufacturerData() const;

    void setRotate180(bool rotate);
    void test();
};
