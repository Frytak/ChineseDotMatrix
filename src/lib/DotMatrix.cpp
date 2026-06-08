#include "DotMatrix.hpp"
#include "BluezDevice.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <sdbus-c++/Message.h>
#include <sdbus-c++/Types.h>
#include <string_view>
#include <utility>
#include <vector>

std::vector<bluez::BluezDevice> DotMatrix::scan(std::shared_ptr<sdbus::IConnection> conn, std::function<void (bluez::BluezDevice device)> callback, std::chrono::milliseconds duration) {
    auto adapter = sdbus::createProxy(*conn, BLUEZ_SERVICE, sdbus::ObjectPath("/org/bluez/hci0"));

    // Set a filter for BLE only
    std::map<std::string, sdbus::Variant> filter;
    filter["Transport"] = sdbus::Variant{std::string("le")};

    adapter->callMethod("SetDiscoveryFilter")
            .onInterface(ADAPTER_IFACE)
            .withArguments(filter);

    auto root = sdbus::createProxy(*conn, BLUEZ_SERVICE, sdbus::ObjectPath("/"));

    // Return saved devices
    ManagedObjects objects;
    std::vector<bluez::BluezDevice> discovered_devices {};
    root->callMethod("GetManagedObjects")
         .onInterface(OBJECT_MANAGER_IFACE)
         .storeResultsTo(objects);
    for (const auto& object : objects) {
        if (!object.second.contains(DEVICE_IFACE)) continue;

        auto device = bluez::BluezDevice(object.first, object.second.at(DEVICE_IFACE));

        auto manufacturer_data = device.getManufacturerData();
        bool matches_expected_data = false;
        for (const auto& [company_id, data] : manufacturer_data.value_or(std::map<std::uint16_t, sdbus::Variant>{})) {
            auto payload = data.get<std::vector<std::uint8_t>>();
            if (std::equal(PAYLOAD.begin(), PAYLOAD.end(), payload.begin())) {
                matches_expected_data = true;
                break;
            }
        }
        if (!matches_expected_data) { continue; }

        discovered_devices.push_back(device);
        callback(device);
    }
    objects.clear();

    adapter->callMethod("StartDiscovery")
            .onInterface(ADAPTER_IFACE);

    // Retrieve devices as they are found
    root->uponSignal("InterfacesAdded")
        .onInterface(OBJECT_MANAGER_IFACE)
        .call([&](InterfacesAddedSignal signal) {
            if (!std::get<1>(signal).contains(DEVICE_IFACE)) { return; }
            auto device = bluez::BluezDevice(std::get<0>(signal), std::get<1>(signal).at(DEVICE_IFACE));

            auto manufacturer_data = device.getManufacturerData();
            bool matches_expected_data = false;
            for (const auto& [company_id, data] : manufacturer_data.value_or(std::map<std::uint16_t, sdbus::Variant>{})) {
                auto payload = data.get<std::vector<std::uint8_t>>();
                if (std::equal(PAYLOAD.begin(), PAYLOAD.end(), payload.begin())) {
                    matches_expected_data = true;
                    break;
                }
            }
            if (!matches_expected_data) { return; }

            discovered_devices.push_back(device);
            callback(device);
        });

    std::this_thread::sleep_for(duration);
    adapter->callMethod("StopDiscovery")
            .onInterface(ADAPTER_IFACE);

    root->callMethod("GetManagedObjects")
         .onInterface(OBJECT_MANAGER_IFACE)
         .storeResultsTo(objects);

    std::erase_if(objects, [](auto object) {
        return !object.second.contains(DEVICE_IFACE);
    });

    return discovered_devices;
}

DotMatrix::DotMatrix(std::shared_ptr<sdbus::IConnection> conn, sdbus::ObjectPath device_path) : DotMatrix(bluez::BluezDeviceProxy(conn, device_path)) {};
DotMatrix::DotMatrix(std::shared_ptr<sdbus::IConnection> conn, bluez::BluezDevice device) : DotMatrix(bluez::BluezDeviceProxy(conn, device)) {};
DotMatrix::DotMatrix(bluez::BluezDeviceProxy device) : conn(device.conn), device(std::move(device)) {};

sdbus::ObjectPath DotMatrix::getObjectPath() const {
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

bool DotMatrix::getConnected() const {
    return device.getConnected();
}

std::optional<std::vector<std::string>> DotMatrix::getUUIDs() const {
    return device.getUUIDs();
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
