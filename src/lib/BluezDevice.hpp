#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/IProxy.h>
#include <sdbus-c++/Types.h>
#include <string>

constexpr auto BLUEZ_SERVICE        = "org.bluez";
constexpr auto ADAPTER_IFACE        = "org.bluez.Adapter1";
constexpr auto DEVICE_IFACE         = "org.bluez.Device1";
constexpr auto GATT_CHAR_IFACE      = "org.bluez.GattCharacteristic1";
constexpr auto OBJECT_MANAGER_IFACE = "org.freedesktop.DBus.ObjectManager";
constexpr auto PROPERTIES_IFACE     = "org.freedesktop.DBus.Properties";

class DotMatrix;

using InterfacesAddedSignal = std::tuple<sdbus::ObjectPath, std::map<std::string, std::map<std::string, sdbus::Variant>>>;
using Properties = std::map<std::string, sdbus::Variant>;
using Characteristics = std::map<std::string, std::shared_ptr<sdbus::IProxy>, std::less<>>;

// Objects returned by GetManagedObjects
using ManagedObjects = std::map<
    sdbus::ObjectPath,
    std::map<std::string, std::map<std::string, sdbus::Variant>>
>;

namespace bluez {
    std::vector<std::string> extract_properties_keys(const Properties& properties);

    enum AddressType {
        PUBLIC,
        RANDOM,
    };

    // Abstract class representing the properties of a device you can get.
    //
    // Documentation for the bluez device properties can be found at
    // https://github.com/bluez/bluez/blob/master/doc/org.bluez.Device.rst#properties
    class BluezDeviceGetInterface {
    public:
        virtual ~BluezDeviceGetInterface() = default;

        virtual std::string getObjectPath() const = 0;
        virtual std::string getAddress() const = 0;
        virtual AddressType getAddressType() const = 0;
        virtual std::optional<std::string> getName() const = 0;
        virtual std::string getAlias() const = 0;
        virtual bool getConnected() const = 0;
        virtual std::optional<std::vector<std::string>> getUUIDs() const = 0;
        virtual std::optional<std::map<std::uint16_t, sdbus::Variant>> getManufacturerData() const = 0;
    };

    // Abstract class representing the properties of a device you can set.
    //
    // Documentation for the bluez device properties can be found at
    // https://github.com/bluez/bluez/blob/master/doc/org.bluez.Device.rst#properties
    class BluezDeviceSetInterface {
    public:
        virtual ~BluezDeviceSetInterface() = default;

        virtual void setAlias(std::string alias) = 0;
    };

    // Abstract class representing the properties of a device you can get or set.
    //
    // Documentation for the properties can be found at
    // https://github.com/bluez/bluez/blob/master/doc/org.bluez.Device.rst#properties
    class BluezDeviceInterface : public BluezDeviceGetInterface, public BluezDeviceSetInterface { };

    // Represents a snapshot of a Bluetooth device with properties, the data is
    // not updated when the device properties change. This is useful for storing
    // device information without needing to maintain a D-Bus connection.
    //
    // Documentation for the bluez device properties can be found at
    // https://github.com/bluez/bluez/blob/master/doc/org.bluez.Device.rst#properties
    class BluezDevice : public BluezDeviceInterface {
    private:
        // Object path of the device
        sdbus::ObjectPath object_path;

        // Properties of the device
        std::string address;
        AddressType address_type;
        std::optional<std::string> name;
        std::string alias;
        bool connected;
        std::optional<std::vector<std::string>> uuids;
        std::optional<std::map<std::uint16_t, sdbus::Variant>> manufacturer_data;

    public:
        static constexpr auto REQUIRED_PROPERTIES = {"Address", "AddressType", /*"Paired", "Bonded",*/ "Connected", /*"Trusted", "Blocked", "WakeAllowed",*/ "Alias", /*"Adapter", "LegacyPairing", "CablePairing", "ServicesResolved", "AdvertisingFlags"*/};

        BluezDevice(sdbus::ObjectPath object_path, Properties properties);

        static std::vector<std::string> return_missing_properties(const std::vector<std::string>& properties);
        static std::vector<std::string> return_missing_properties(const Properties& properties);

        template <typename T>
        static void validate_property_type(const Properties& properties, std::string property_name, std::string expected_type);

        std::string getObjectPath() const override;
        std::string getAddress() const override;
        AddressType getAddressType() const override;
        std::optional<std::string> getName() const override;
        std::string getAlias() const override;
        bool getConnected() const override;
        std::optional<std::vector<std::string>> getUUIDs() const override;
        std::optional<std::map<std::uint16_t, sdbus::Variant>> getManufacturerData() const override;

        void setAlias(std::string alias) override;
    };

    // Represents a Bluetooth device with properties that can be read and
    // written, this class maintains a D-Bus connection and proxies to the
    // device, so the properties are always up to date. This is useful for
    // interacting with devices and performing operations like connecting,
    // reading/writing characteristics, etc.
    //
    // Documentation for the bluez device properties can be found at
    // https://github.com/bluez/bluez/blob/master/doc/org.bluez.Device.rst#properties
    class BluezDeviceProxy : public BluezDeviceInterface {
    private:
        // D-Bus connection
        std::shared_ptr<sdbus::IConnection> conn;

        // Object path of the device
        sdbus::ObjectPath object_path;

        // Proxy for the device
        std::unique_ptr<sdbus::IProxy> device;

        // Cached characteristic proxies, keyed by UUID
        Characteristics characteristics;

        struct ConnectionState {
            std::mutex is_connected_mutex;
            std::condition_variable is_connected_cv;
            bool is_connected;
        };

        std::shared_ptr<ConnectionState> connection_state;



        // Helper to get a characteristic proxy by UUID, with caching
        std::shared_ptr<sdbus::IProxy>& getCharacteristic(std::string_view uuid);

        void registerConnectionListener();

    public:
        BluezDeviceProxy(std::shared_ptr<sdbus::IConnection> conn, sdbus::ObjectPath object_path);
        BluezDeviceProxy(std::shared_ptr<sdbus::IConnection> conn, bluez::BluezDevice device);

        //BluezDeviceProxy(const BluezDeviceProxy&) = default;
        //BluezDeviceProxy& operator=(const BluezDeviceProxy&) = default;
        BluezDeviceProxy(BluezDeviceProxy&&) noexcept = default;
        BluezDeviceProxy& operator=(BluezDeviceProxy&&) noexcept = default;

        ~BluezDeviceProxy() noexcept;

        // Establish a connection to the device, this is required before you can read/write
        void connect();

        // Disconnect from the device
        void disconnect();

        // Write data to a characteristic by UUID
        void write(std::string_view uuid, std::vector<uint8_t> data);

        // Read data from a characteristic by UUID
        std::vector<uint8_t> read(std::string_view uuid);

        std::string getObjectPath() const override;
        std::string getAddress() const override;
        AddressType getAddressType() const override;
        std::optional<std::string> getName() const override;
        std::string getAlias() const override;
        bool getConnected() const override;
        std::optional<std::vector<std::string>> getUUIDs() const override;
        std::optional<std::map<std::uint16_t, sdbus::Variant>> getManufacturerData() const override;

        void setAlias(std::string alias) override;

        friend DotMatrix;
    };

    namespace bluez_device {
        class MissingPropertiesError : public std::logic_error {
        private:
            const std::vector<std::string> provided_properties;
            const std::vector<std::string> missing_properties;

            static std::string build_message(const std::vector<std::string>& missing_properties);

            MissingPropertiesError(const std::vector<std::string>& provided_propertiesd, const std::vector<std::string>& missing_properties);

        public:
            explicit MissingPropertiesError(const std::vector<std::string>& provided_properties);
            explicit MissingPropertiesError(const Properties& provided_properties);
        };

        class InvalidPropertyTypeError : public std::logic_error {
        private:
            const std::string property_name;
            const std::string expected_type;
            const std::string provided_type;

        public:
            explicit InvalidPropertyTypeError(std::string property_name, std::string expected_type, std::string provided_type);
        };

        class InvalidPropertyValueError : public std::logic_error {
        private:
            const std::string property_name;
            const sdbus::Variant provided_value;
        public:
            explicit InvalidPropertyValueError(std::string property_name, sdbus::Variant provided_value);
        };
    }
}
