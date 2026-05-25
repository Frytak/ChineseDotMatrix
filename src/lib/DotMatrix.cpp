#include "DotMatrix.hpp"
#include <chrono>
#include <cstdint>
#include <functional>
#include <sdbus-c++/Message.h>
#include <sdbus-c++/Types.h>
#include <string_view>
#include <utility>
#include <vector>

std::vector<bluez::BluezDevice> DotMatrix::scan(std::shared_ptr<sdbus::IConnection> conn, std::function<void (bluez::BluezDevice device)> callback, std::chrono::milliseconds duration) {
    auto adapter = sdbus::createProxy(*conn, BLUEZ_SERVICE, "/org/bluez/hci0");

    // Set a filter for BLE only
    std::map<std::string, sdbus::Variant> filter;
    filter["Transport"] = sdbus::Variant{std::string("le")};

    adapter->callMethod("SetDiscoveryFilter")
            .onInterface(ADAPTER_IFACE)
            .withArguments(filter);

    auto root = sdbus::createProxy(*conn, BLUEZ_SERVICE, "/");

    // Return saved devices
    ManagedObjects objects;
    root->callMethod("GetManagedObjects")
         .onInterface(OBJECT_MANAGER_IFACE)
         .storeResultsTo(objects);
    for (const auto& object : objects) {
         if (!object.second.contains(DEVICE_IFACE)) continue;
         callback(bluez::BluezDevice(object.first, object.second.at(DEVICE_IFACE)));
    }
    objects.clear();

    adapter->callMethod("StartDiscovery")
            .onInterface(ADAPTER_IFACE);

    // Retrieve devices as they are found
    root->uponSignal("InterfacesAdded")
        .onInterface(OBJECT_MANAGER_IFACE)
        .call([&callback](InterfacesAddedSignal signal) {
            if (!std::get<1>(signal).contains(DEVICE_IFACE)) return;
            callback(bluez::BluezDevice(std::get<0>(signal), std::get<1>(signal).at(DEVICE_IFACE)));
        });
    root->finishRegistration();
    conn->enterEventLoopAsync();

    std::this_thread::sleep_for(duration);
    adapter->callMethod("StopDiscovery")
            .onInterface(ADAPTER_IFACE);

    root->callMethod("GetManagedObjects")
         .onInterface(OBJECT_MANAGER_IFACE)
         .storeResultsTo(objects);

    std::erase_if(objects, [](auto object) {
        return !object.second.contains(DEVICE_IFACE);
    });

    conn->leaveEventLoop();
    // TODO: return all discovered devices
    return {};
}

DotMatrix::DotMatrix(std::shared_ptr<sdbus::IConnection> conn, sdbus::ObjectPath device_path) : DotMatrix(bluez::BluezDeviceProxy(conn, device_path)) {};
DotMatrix::DotMatrix(std::shared_ptr<sdbus::IConnection> conn, bluez::BluezDevice device) : DotMatrix(bluez::BluezDeviceProxy(conn, device)) {};
DotMatrix::DotMatrix(bluez::BluezDeviceProxy device) : conn(device.conn), device(device) {};

std::string DotMatrix::getObjectPath() const {
    return device.getObjectPath();
}

std::string DotMatrix::getAddress() const {
    return device.getAddress();
}

bluez::AddressType DotMatrix::getAddressType() const {
    return device.getAddressType();
}

std::optional<std::string> DotMatrix::getName() const {
    return device.getName();
}

std::string DotMatrix::getAlias() const {
    return device.getAlias();
}

std::optional<std::map<std::uint16_t, sdbus::Variant>> DotMatrix::getManufacturerData() const {
    return device.getManufacturerData();
}

void DotMatrix::connect() {
    device.connect();
}

void DotMatrix::disconnect() {
    device.disconnect();
}

void DotMatrix::setRotate180(bool rotated) {
    device.write(UUID_WRITE_CHAR, {0x05, 0x00, 0x06, 0x80, rotated});
}

void DotMatrix::setDrawingMode() {
    device.write(UUID_WRITE_CHAR, {0x05, 0x00, 0x04, 0x01, 0x01});
}

void DotMatrix::setPixel(std::uint8_t x, std::uint8_t y, Color color) {
    device.write(UUID_WRITE_CHAR, {0x0a, 0x00, 0x05, 0x01, 0x00, color.r, color.g, color.b, x, y});
}
