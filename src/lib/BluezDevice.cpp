#include "BluezDevice.hpp"
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <sdbus-c++/Types.h>

namespace bluez {
    std::vector<std::string> extract_properties_keys(const Properties& properties) {
        std::vector<std::string> keys;
        keys.reserve(properties.size()); 
        
        for (const auto& [key, value] : properties) { 
            keys.push_back(key);
        }
        
        return keys;
    }

    BluezDevice::BluezDevice(sdbus::ObjectPath object_path, std::map<std::string, sdbus::Variant> properties) : object_path(object_path), properties(properties) {
        std::vector<std::string> missing_properties = BluezDevice::return_missing_properties(properties); 
        if (!missing_properties.empty()) {
            throw bluez_device::MissingPropertiesError(properties);
        }

        // TODO: Validate property values
    };

    std::vector<std::string> BluezDevice::return_missing_properties(const std::vector<std::string>& properties) {
        std::vector<std::string> missing_properties;

        for (const auto& required_property : BluezDevice::REQUIRED_PROPERTIES) {
            if (std::find(properties.begin(), properties.end(), required_property) == properties.end()) {
                missing_properties.push_back(required_property);
            }
        }

        return missing_properties;
    }

    std::vector<std::string> BluezDevice::return_missing_properties(const Properties& properties) {
        return return_missing_properties(extract_properties_keys(properties));
    }

    std::string BluezDevice::getObjectPath() const {
        return object_path;
    }

    std::string BluezDevice::getAddress() const {
        return properties.at("Address").get<std::string>();
    }

    AddressType BluezDevice::getAddressType() const {
        return properties.at("AddressType").get<std::string>() == "public" ? PUBLIC : RANDOM;
    }

    std::optional<std::string> BluezDevice::getName() const {
        if (properties.contains("Name")) {
            return properties.at("Name").get<std::string>();
        }
        return std::nullopt;
    }

    std::string BluezDevice::getAlias() const {
        return properties.at("Alias").get<std::string>();
    }

    std::optional<std::map<std::uint16_t, sdbus::Variant>> BluezDevice::getManufacturerData() const {
        if (properties.contains("ManufacturerData")) {
            return properties.at("ManufacturerData").get<std::map<std::uint16_t, sdbus::Variant>>();
        }
        return std::nullopt;
    }



    BluezDeviceProxy::BluezDeviceProxy(std::shared_ptr<sdbus::IConnection> conn, sdbus::ObjectPath object_path)
        : conn(conn),
          object_path(object_path),
          device(sdbus::createProxy(*conn, BLUEZ_SERVICE, object_path)),
          characteristics(Characteristics())
    { }

    BluezDeviceProxy::BluezDeviceProxy(std::shared_ptr<sdbus::IConnection> conn, bluez::BluezDevice device) : BluezDeviceProxy(conn, device.getObjectPath()) {}

    std::shared_ptr<sdbus::IProxy>& BluezDeviceProxy::getCharacteristic(std::string_view uuid) {
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
            if (std::string(path).find(object_path) == std::string::npos) continue;

            auto charUUID = interfaces.at(GATT_CHAR_IFACE).at("UUID").get<std::string>();
            if (charUUID == uuid) {
                auto [inserted, _] = characteristics.emplace(std::string(uuid), sdbus::createProxy(*conn, BLUEZ_SERVICE, path));
                return inserted->second;
            }
        }

        throw "Characteristic UUID not found: " + std::string(uuid);
    }

    void BluezDeviceProxy::connect() {
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

    void BluezDeviceProxy::disconnect() {
        device->callMethod("Disconnect").onInterface(DEVICE_IFACE);
    }

    void BluezDeviceProxy::write(std::string_view uuid, std::vector<uint8_t> data) {
        auto characteristic = sdbus::createProxy(*conn, BLUEZ_SERVICE, getCharacteristic(uuid)->getObjectPath());
        std::map<std::string, sdbus::Variant> options;

        characteristic->callMethod("WriteValue")
                       .onInterface(GATT_CHAR_IFACE)
                       .withArguments(data, options);
    }

    std::vector<uint8_t> BluezDeviceProxy::read(std::string_view uuid) {
        auto characteristic = sdbus::createProxy(*conn, BLUEZ_SERVICE, getCharacteristic(uuid)->getObjectPath());
        std::map<std::string, sdbus::Variant> options;

        std::vector<uint8_t> data;
        characteristic->callMethod("ReadValue")
                       .onInterface(GATT_CHAR_IFACE)
                       .withArguments(options)
                       .storeResultsTo(data);

        return data;
    }

    std::string BluezDeviceProxy::getObjectPath() const {
        return object_path;
    }

    std::string BluezDeviceProxy::getAddress() const {
        sdbus::Variant var;
        device->callMethod("Get")
              .onInterface(PROPERTIES_IFACE)
              .withArguments(DEVICE_IFACE, "Address")
              .storeResultsTo(var);

        std::string address = var.get<std::string>();
        return address;
    }

    AddressType BluezDeviceProxy::getAddressType() const {
        sdbus::Variant var;
        device->callMethod("Get")
              .onInterface(PROPERTIES_IFACE)
              .withArguments(DEVICE_IFACE, "AddressType")
              .storeResultsTo(var);

        std::string address_type = var.get<std::string>();
        return address_type == "public" ? PUBLIC : RANDOM;
    }

    // TODO: Check if exists
    std::optional<std::string> BluezDeviceProxy::getName() const {
        sdbus::Variant var;
        device->callMethod("Get")
              .onInterface(PROPERTIES_IFACE)
              .withArguments(DEVICE_IFACE, "Name")
              .storeResultsTo(var);

        std::string name = var.get<std::string>();
        return name;
    }

    std::string BluezDeviceProxy::getAlias() const {
        sdbus::Variant var;
        device->callMethod("Get")
              .onInterface(PROPERTIES_IFACE)
              .withArguments(DEVICE_IFACE, "Alias")
              .storeResultsTo(var);

        std::string alias = var.get<std::string>();
        return alias;
    }

    std::optional<std::map<std::uint16_t, sdbus::Variant>> BluezDeviceProxy::getManufacturerData() const {
        sdbus::Variant var;
        device->callMethod("Get")
              .onInterface(PROPERTIES_IFACE)
              .withArguments(DEVICE_IFACE, "ManufacturerData")
              .storeResultsTo(var);

        std::map<std::uint16_t, sdbus::Variant> manufacturer_data = var.get<std::map<std::uint16_t, sdbus::Variant>>();
        return manufacturer_data;
    }

    void BluezDeviceProxy::setAlias(std::string alias) {
        device->callMethod("Set")
              .onInterface(PROPERTIES_IFACE)
              .withArguments(DEVICE_IFACE, "Alias", alias);
    }

    namespace bluez_device {
        std::string MissingPropertiesError::build_message(const std::vector<std::string>& missing_properties) {
            if (missing_properties.empty()) {
                return "Missing required properties: (none)";
            }
            
            return "Missing required properties: " + std::accumulate(
                std::next(missing_properties.begin()), 
                missing_properties.end(), 
                missing_properties.front(),
                [](const std::string& a, const std::string& b) {
                    return a + ", " + b;
                }
            );
        }

        MissingPropertiesError::MissingPropertiesError(const std::vector<std::string>& provided_properties, const std::vector<std::string>& missing_properties)
            : std::logic_error(build_message(missing_properties))
            , provided_properties(provided_properties)
            , missing_properties(missing_properties)
        { }

        MissingPropertiesError::MissingPropertiesError(const std::vector<std::string>& provided_properties)
            : MissingPropertiesError(provided_properties, BluezDevice::return_missing_properties(provided_properties))
        { }

        MissingPropertiesError::MissingPropertiesError(const Properties& provided_properties)
            : MissingPropertiesError(extract_properties_keys(provided_properties))
        { }
    }
}
