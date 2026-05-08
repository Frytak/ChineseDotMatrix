#include "./lib.hpp"
#include <iostream>
#include <string_view>

ManagedObjects DotMatrix::scan(std::shared_ptr<sdbus::IConnection> conn) {
    auto adapter = sdbus::createProxy(*conn, BLUEZ_SERVICE, "/org/bluez/hci0");

    // Set a filter for BLE only
    std::map<std::string, sdbus::Variant> filter;
    filter["Transport"] = sdbus::Variant{std::string("le")};

    adapter->callMethod("SetDiscoveryFilter")
            .onInterface(ADAPTER_IFACE)
            .withArguments(filter);

    adapter->callMethod("StartDiscovery")
            .onInterface(ADAPTER_IFACE);

    // TODO: Use InterfacesAdded signal to get devices as they are found
    std::cout << "Scanning...\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    adapter->callMethod("StopDiscovery")
            .onInterface(ADAPTER_IFACE);

    auto root = sdbus::createProxy(*conn, BLUEZ_SERVICE, "/");

    ManagedObjects objects;
    root->callMethod("GetManagedObjects")
         .onInterface(OBJECT_MANAGER_IFACE)
         .storeResultsTo(objects);

    std::erase_if(objects, [](auto object) {
        return !object.second.contains(DEVICE_IFACE);
    });

    return objects;
}

DotMatrix::DotMatrix(std::shared_ptr<sdbus::IConnection> conn, sdbus::ObjectPath device_path)
    : conn(conn),
      device(sdbus::createProxy(*conn, BLUEZ_SERVICE, device_path)),
      device_path(device_path)
{
    device->callMethod("Connect").onInterface(DEVICE_IFACE);

    // TODO: Use PropertiesChanged signal
    for (int i = 0; i < 30; i++) {
        sdbus::Variant resolved;
        device->callMethod("Get")
               .onInterface(PROPERTIES_IFACE)
               .withArguments(DEVICE_IFACE, std::string("ServicesResolved"))
               .storeResultsTo(resolved);

        if (resolved.get<bool>()) {
            std::cout << "GATT ready\n";
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

std::unique_ptr<sdbus::IProxy>& DotMatrix::getCharacteristic(std::string_view uuid) {
    if (characteristics.contains(uuid)) {
        return characteristics.find(uuid)->second;
    }

    auto root = sdbus::createProxy(*conn, BLUEZ_SERVICE, "/");
    ManagedObjects objects;
    root->callMethod("GetManagedObjects")
         .onInterface(OBJECT_MANAGER_IFACE)
         .storeResultsTo(objects);

    for (auto& [path, interfaces] : objects) {
        if (!interfaces.contains(GATT_CHAR_IFACE)) continue;
        if (std::string(path).find(device_path) == std::string::npos) continue;

        auto charUUID = interfaces.at(GATT_CHAR_IFACE).at("UUID").get<std::string>();
        if (charUUID == uuid) {
            auto [inserted, _] = characteristics.emplace(std::string(uuid), sdbus::createProxy(*conn, BLUEZ_SERVICE, path));
            return inserted->second;
        }
    }

    throw "Characteristic UUID not found: " + std::string(uuid);
}

void DotMatrix::write(std::string_view uuid, std::vector<uint8_t> data) {
    auto characteristic = sdbus::createProxy(*conn, BLUEZ_SERVICE, getCharacteristic(uuid)->getObjectPath());
    std::map<std::string, sdbus::Variant> options;

    characteristic->callMethod("WriteValue")
                   .onInterface(GATT_CHAR_IFACE)
                   .withArguments(data, options);
}

std::vector<uint8_t> DotMatrix::read(std::string_view uuid) {
    auto characteristic = sdbus::createProxy(*conn, BLUEZ_SERVICE, getCharacteristic(uuid)->getObjectPath());
    std::map<std::string, sdbus::Variant> options;

    std::vector<uint8_t> data;
    characteristic->callMethod("ReadValue")
                   .onInterface(GATT_CHAR_IFACE)
                   .withArguments(options)
                   .storeResultsTo(data);

    return data;
}

void DotMatrix::setRotate180(bool rotate) {
    write(UUID_WRITE_CHAR, {5, 0, 6, 128, rotate});
}
