#include "BluezDevice.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <sdbus-c++/Error.h>
#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/Types.h>
#include "LoggerInternal.hpp"

namespace bluez {
    std::vector<std::string> extract_properties_keys(const Properties& properties) {
        std::vector<std::string> keys;
        keys.reserve(properties.size()); 
        
        for (const auto& [key, value] : properties) { 
            keys.push_back(key);
        }
        
        return keys;
    }

    BluezDevice::BluezDevice(sdbus::ObjectPath object_path, std::map<std::string, sdbus::Variant> properties) : object_path(object_path) {
        // Validate required properties
        std::vector<std::string> missing_properties = BluezDevice::return_missing_properties(properties); 
        if (!missing_properties.empty()) { throw bluez_device::MissingPropertiesError(properties); }

        // Validate property types
        BluezDevice::validate_property_type<std::string>(properties, "Address", "s");
        BluezDevice::validate_property_type<std::string>(properties, "AddressType", "s");
        BluezDevice::validate_property_type<std::string>(properties, "Name", "s");
        BluezDevice::validate_property_type<std::string>(properties, "Alias", "s");
        BluezDevice::validate_property_type<bool>(properties, "Connected", "b");
        BluezDevice::validate_property_type<std::vector<std::string>>(properties, "UUIDs", "as");
        BluezDevice::validate_property_type<std::map<std::uint16_t, sdbus::Variant>>(properties, "ManufacturerData", "a{qv}");

        // Validate and assign property values
        address = properties.at("Address").get<std::string>();

        std::string raw_address_type = properties.at("AddressType").get<std::string>();
        if (raw_address_type != "public" && raw_address_type != "random") {
            throw bluez_device::InvalidPropertyValueError("AddressType", properties.at("AddressType"));
        }
        address_type = raw_address_type == "public" ? PUBLIC : RANDOM;

        name = properties.contains("Name") ? static_cast<std::optional<std::string>>(properties.at("Name").get<std::string>()) : std::nullopt;
        alias = properties.at("Alias").get<std::string>();
        connected = properties.at("Connected").get<bool>();
        uuids = properties.contains("UUIDs") ? static_cast<std::optional<std::vector<std::string>>>(properties.at("UUIDs").get<std::vector<std::string>>()) : std::nullopt;
        manufacturer_data = properties.contains("ManufacturerData") ? static_cast<std::optional<std::map<std::uint16_t, sdbus::Variant>>>(properties.at("ManufacturerData").get<std::map<std::uint16_t, sdbus::Variant>>()) : std::nullopt;
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

    template <typename T>
    void BluezDevice::validate_property_type(const Properties& properties, std::string property_name, std::string expected_type) {
        if (!properties.contains(property_name)) { return; }
        if (!properties.at(property_name).containsValueOfType<T>()) {
            throw bluez_device::InvalidPropertyTypeError(property_name, properties.at(property_name).peekValueType(), expected_type);
        }
    }

    sdbus::ObjectPath BluezDevice::getObjectPath() const { return object_path; }
    std::string BluezDevice::getAddress() const { return address; }
    AddressType BluezDevice::getAddressType() const { return address_type; }
    std::optional<std::string> BluezDevice::getName() const { return name; }
    std::string BluezDevice::getAlias() const { return alias; }
    bool BluezDevice::getConnected() const { return connected; }
    std::optional<std::vector<std::string>> BluezDevice::getUUIDs() const { return uuids; }
    std::optional<std::map<std::uint16_t, sdbus::Variant>> BluezDevice::getManufacturerData() const { return manufacturer_data; }

    void BluezDevice::setAlias(std::string alias) { this->alias = alias; }



    BluezDeviceProxy::BluezDeviceProxy(std::shared_ptr<sdbus::IConnection> conn, sdbus::ObjectPath object_path)
        : conn(conn)
        , object_path(object_path)
        , device(sdbus::createProxy(*conn, BLUEZ_SERVICE, object_path))
        , characteristics(Characteristics())
        , connection_state(std::make_shared<ConnectionState>())
    {
        registerConnectionListener();
    }

    BluezDeviceProxy::BluezDeviceProxy(std::shared_ptr<sdbus::IConnection> conn, bluez::BluezDevice device) : BluezDeviceProxy(conn, device.getObjectPath()) {}

    BluezDeviceProxy::~BluezDeviceProxy() noexcept {
        if (!device) {
            CDM_DEBUG("Destroying BluezDeviceProxy.");
            return;
        }

        if (!getConnected()) {
            CDM_DEBUG("Destroying BluezDeviceProxy (`{}`).", object_path.c_str());
            return;
        }

        try {
            disconnect();
        } catch (const sdbus::Error& err) {
            CDM_ERR("Failed to disconnect from device (`{}`) during BluezDeviceProxy destruction. {}", object_path.c_str(), err.what());
        }
    }

    std::shared_ptr<sdbus::IProxy>& BluezDeviceProxy::getCharacteristic(std::string_view uuid) {
        if (characteristics.contains(uuid)) {
            return characteristics.find(uuid)->second;
        }

        auto root = sdbus::createProxy(*conn, BLUEZ_SERVICE, sdbus::ObjectPath("/"));
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

    void BluezDeviceProxy::registerConnectionListener() {
        device->uponSignal("PropertiesChanged")
            .onInterface(PROPERTIES_IFACE)
            .call([connection_state = connection_state](const std::string& interface_name, const std::map<std::string, sdbus::Variant>& changed_properties, const std::vector<std::string>& /*invalidated_properties*/) {
                if (interface_name != DEVICE_IFACE) { return; }

                auto it = changed_properties.find("Connected");
                if (it != changed_properties.end()) {
                    bool is_connected = it->second.get<bool>();

                    std::lock_guard<std::mutex> lock(connection_state->is_connected_mutex);
                    connection_state->is_connected = is_connected;
                    
                    if (is_connected) {
                        connection_state->is_connected_cv.notify_all();
                    }
                }
            });
    }

    void BluezDeviceProxy::connect() {
        CDM_INFO("Connecting to `{}`.", object_path.c_str());

        sdbus::Variant resolved;
        device->callMethod("Get")
              .onInterface(PROPERTIES_IFACE)
              .withArguments(DEVICE_IFACE, std::string("ServicesResolved"))
              .storeResultsTo(resolved);

        {
            std::lock_guard<std::mutex> lock(connection_state->is_connected_mutex);
            connection_state->is_connected = resolved.get<bool>();
        }

        if (connection_state->is_connected) {
            CDM_INFO("Device (`{}`) is already connected to and GATT services are resolved.", object_path.c_str());
            return;
        }

        try {
            device->callMethod("Connect").onInterface(DEVICE_IFACE);
        } catch (const sdbus::Error& err) {
            CDM_ERR("Failed to connect to device (`{}`). {}", object_path.c_str(), err.what());
            throw err;
        }

        std::unique_lock<std::mutex> lock(connection_state->is_connected_mutex);
        bool success = connection_state->is_connected_cv.wait_for(lock, std::chrono::seconds(5), [this]() {
            return connection_state->is_connected;
        });

        if (!success) {
            CDM_WARN("Timeout waiting for device (`{}`) to connect and resolve GATT services.", object_path.c_str());

            try {
                device->callMethod("Disconnect").onInterface(DEVICE_IFACE);
            } catch (...) {}

            connection_state->is_connected = false;
        } else {
            CDM_INFO("Successfully connected to device (`{}`) and resolved GATT services.", object_path.c_str());
        }
    }

    void BluezDeviceProxy::disconnect() {
        CDM_INFO("Disconnecting from device (`{}`).", object_path.c_str());
        try {
            device->callMethod("Disconnect").onInterface(DEVICE_IFACE);
        } catch (const sdbus::Error& err) {
            CDM_ERR("Failed to disconnect from device (`{}`). {}", object_path.c_str(), err.what());
            throw err;
        }
        CDM_INFO("Successfully disconnected from device (`{}`).", object_path.c_str());
    }

    void BluezDeviceProxy::write(std::string_view uuid, std::vector<uint8_t> data) {
        if (!getConnected()) {
            return;
        }
        auto characteristic = sdbus::createProxy(*conn, BLUEZ_SERVICE, getCharacteristic(uuid)->getObjectPath());
        std::map<std::string, sdbus::Variant> options;

        try {
            characteristic->callMethod("WriteValue")
                           .onInterface(GATT_CHAR_IFACE)
                           .withArguments(data, options);
        } catch (const sdbus::Error& err) {
            CDM_ERR("Failed to write to characteristic `{}` on device `{}`. {}", uuid.data(), object_path.c_str(), err.what());
            throw err;
        }
    }

    std::vector<uint8_t> BluezDeviceProxy::read(std::string_view uuid) {
        auto characteristic = sdbus::createProxy(*conn, BLUEZ_SERVICE, getCharacteristic(uuid)->getObjectPath());
        std::map<std::string, sdbus::Variant> options;

        std::vector<uint8_t> data;

        try {
            characteristic->callMethod("ReadValue")
                           .onInterface(GATT_CHAR_IFACE)
                           .withArguments(options)
                           .storeResultsTo(data);
        } catch (const sdbus::Error& err) {
            CDM_ERR("Failed to read from characteristic `{}` on device `{}`. {}", uuid.data(), object_path.c_str(), err.what());
            throw err;
        }

        return data;
    }

    sdbus::ObjectPath BluezDeviceProxy::getObjectPath() const {
        return object_path;
    }

    std::string BluezDeviceProxy::getAddress() const {
        CDM_DEBUG("Getting the `Address` property for device (`{}`).", object_path.c_str());
        sdbus::Variant var;

        try {
            device->callMethod("Get")
                  .onInterface(PROPERTIES_IFACE)
                  .withArguments(DEVICE_IFACE, "Address")
                  .storeResultsTo(var);
        } catch (const sdbus::Error& err) {
            CDM_ERR("Failed to get `Address` property for device (`{}`). {}", object_path.c_str(), err.what());
            throw err;
        }

        auto address = var.get<std::string>();
        CDM_DEBUG("Got the `Address` property (`{}`) for device (`{}`).", address, object_path.c_str());
        return address;
    }

    AddressType BluezDeviceProxy::getAddressType() const {
        CDM_DEBUG("Getting the `AddressType` property for device (`{}`).", object_path.c_str());
        sdbus::Variant var;

        try {
            device->callMethod("Get")
                  .onInterface(PROPERTIES_IFACE)
                  .withArguments(DEVICE_IFACE, "AddressType")
                  .storeResultsTo(var);
        } catch (const sdbus::Error& err) {
            CDM_ERR("Failed to get `AddressType` property for device (`{}`). {}", object_path.c_str(), err.what());
            throw err;
        }

        auto address_type = var.get<std::string>();
        CDM_DEBUG("Got the `AddressType` property (`{}`) for device (`{}`).", address_type, object_path.c_str());
        return address_type == "public" ? PUBLIC : RANDOM;
    }

    std::optional<std::string> BluezDeviceProxy::getName() const {
        CDM_DEBUG("Getting the `Name` property for device (`{}`).", object_path.c_str());
        sdbus::Variant var;

        try {
            device->callMethod("Get")
                  .onInterface(PROPERTIES_IFACE)
                  .withArguments(DEVICE_IFACE, "Name")
                  .storeResultsTo(var);
        } catch (const sdbus::Error& err) {
            if (err.getName() == "org.freedesktop.DBus.Error.UnknownProperty") {
                CDM_WARN("Device (`{}`) does not have a `Name` property.", object_path.c_str());
                return std::nullopt;
            } else {
                CDM_ERR("Failed to get `Name` property for device (`{}`). {}", object_path.c_str(), err.what());
                throw err;
            }
        }

        auto name = var.get<std::string>();
        CDM_DEBUG("Got the `Name` property (`{}`) for device (`{}`).", name, object_path.c_str());
        return name;
    }

    std::string BluezDeviceProxy::getAlias() const {
        CDM_DEBUG("Getting the `Alias` property for device (`{}`).", object_path.c_str());
        sdbus::Variant var;

        try {
            device->callMethod("Get")
                  .onInterface(PROPERTIES_IFACE)
                  .withArguments(DEVICE_IFACE, "Alias")
                  .storeResultsTo(var);
        } catch (const sdbus::Error& err) {
            CDM_ERR("Failed to get `Alias` property for device (`{}`). {}", object_path.c_str(), err.what());
            throw err;
        }

        auto alias = var.get<std::string>();
        CDM_DEBUG("Got the `Alias` property (`{}`) for device (`{}`).", alias, object_path.c_str());
        return alias;
    }

    bool BluezDeviceProxy::getConnected() const {
        CDM_DEBUG("Getting the `Connected` property for device (`{}`).", object_path.c_str());
        sdbus::Variant var;

        try {
            device->callMethod("Get")
                  .onInterface(PROPERTIES_IFACE)
                  .withArguments(DEVICE_IFACE, "Connected")
                  .storeResultsTo(var);
        } catch (const sdbus::Error& err) {
            CDM_ERR("Failed to get `Connected` property for device (`{}`). {}", object_path.c_str(), err.what());
            throw err;
        }

        auto connected = var.get<bool>();
        CDM_DEBUG("Got the `Connected` property (`{}`) for device (`{}`).", connected, object_path.c_str());
        return connected;
    }

    std::optional<std::vector<std::string>> BluezDeviceProxy::getUUIDs() const {
        CDM_DEBUG("Getting the `UUIDs` property for device (`{}`).", object_path.c_str());
        sdbus::Variant var;

        try {
            device->callMethod("Get")
                  .onInterface(PROPERTIES_IFACE)
                  .withArguments(DEVICE_IFACE, "UUIDs")
                  .storeResultsTo(var);
        } catch (const sdbus::Error& err) {
            if (err.getName() == "org.freedesktop.DBus.Error.UnknownProperty") {
                CDM_WARN("Device (`{}`) does not have a `UUIDs` property.", object_path.c_str());
                return std::nullopt;
            } else {
                CDM_ERR("Failed to get `UUIDs` property for device (`{}`). {}", object_path.c_str(), err.what());
                throw err;
            }
        }

        auto uuids = var.get<std::vector<std::string>>();
        CDM_DEBUG("Got the `UUIDs` property for device (`{}`).", object_path.c_str());
        return uuids;
    }

    std::optional<std::map<std::uint16_t, sdbus::Variant>> BluezDeviceProxy::getManufacturerData() const {
        CDM_DEBUG("Getting the `ManufacturerData` property for device (`{}`).", object_path.c_str());
        sdbus::Variant var;

        try {
            device->callMethod("Get")
                  .onInterface(PROPERTIES_IFACE)
                  .withArguments(DEVICE_IFACE, "ManufacturerData")
                  .storeResultsTo(var);
        } catch (const sdbus::Error& err) {
            if (err.getName() == "org.freedesktop.DBus.Error.UnknownProperty") {
                CDM_WARN("Device (`{}`) does not have a `ManufacturerData` property.", object_path.c_str());
                return std::nullopt;
            } else {
                CDM_ERR("Failed to get `ManufacturerData` property for device (`{}`). {}", object_path.c_str(), err.what());
                throw err;
            }
        }

        auto manufacturer_data = var.get<std::map<std::uint16_t, sdbus::Variant>>();
        CDM_DEBUG("Got the `ManufacturerData` property for device (`{}`).", object_path.c_str());
        return manufacturer_data;

    }

    void BluezDeviceProxy::setAlias(std::string alias) {
        CDM_DEBUG("Setting the `Alias` property for device (`{}`) to `{}`.", object_path.c_str(), alias);

        try {
            device->callMethod("Set")
                  .onInterface(PROPERTIES_IFACE)
                  .withArguments(DEVICE_IFACE, "Alias", alias);
        } catch (const sdbus::Error& err) {
            CDM_ERR("Failed to set `Alias` property for device (`{}`). {}", object_path.c_str(), err.what());
            throw err;
        }

        CDM_DEBUG("Set the `Alias` property for device (`{}`) to `{}`.", object_path.c_str(), alias);
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


        InvalidPropertyTypeError::InvalidPropertyTypeError(std::string property_name, std::string expected_type, std::string provided_type)
            : std::logic_error("Invalid type for property `" + property_name + "`. Expected `" + expected_type + "`, but got `" + provided_type + "`.")
            , property_name(property_name)
            , expected_type(expected_type)
        { }

        InvalidPropertyValueError::InvalidPropertyValueError(std::string property_name, sdbus::Variant provided_value)
            : std::logic_error("Invalid value for property `" + property_name + "`.")
            , property_name(property_name)
            , provided_value(provided_value)
        { }
    }
}
