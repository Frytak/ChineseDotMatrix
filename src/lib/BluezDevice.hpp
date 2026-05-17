#pragma once

#include <cstdint>
#include <map>
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
using Characteristics = std::map<std::string, std::shared_ptr<sdbus::IProxy>, std::less<>>;

// Objects returned by GetManagedObjects
using ManagedObjects = std::map<
    sdbus::ObjectPath,
    std::map<std::string, std::map<std::string, sdbus::Variant>>
>;

namespace bluez {
    enum AddressType {
        PUBLIC,
        RANDOM,
    };

    class BluezDeviceGetInterface {
    public:
        virtual ~BluezDeviceGetInterface() = default;

        virtual std::string getObjectPath() const = 0;
        virtual std::string getAddress() const = 0;
        virtual AddressType getAddressType() const = 0;
        virtual std::optional<std::string> getName() const = 0;
        virtual std::string getAlias() const = 0;
        virtual std::optional<std::map<std::uint16_t, sdbus::Variant>> getManufacturerData() const = 0;
    };

    class BluezDeviceSetInterface {
    public:
        virtual ~BluezDeviceSetInterface() = default;

        virtual void setAlias(std::string alias) = 0;
    };

    class BluezDeviceInterface : public BluezDeviceGetInterface, public BluezDeviceSetInterface { };

    class BluezDevice : public BluezDeviceGetInterface {
    private:
        // Object path of the device
        sdbus::ObjectPath object_path;

        // Properties of the device
        std::map<std::string, sdbus::Variant> properties;

    public:
        BluezDevice(sdbus::ObjectPath object_path, std::map<std::string, sdbus::Variant> properties);

        std::string getObjectPath() const override;
        std::string getAddress() const override;
        AddressType getAddressType() const override;
        std::optional<std::string> getName() const override;
        std::string getAlias() const override;
        std::optional<std::map<std::uint16_t, sdbus::Variant>> getManufacturerData() const override;
    };

    class BluezDeviceProxy : public BluezDeviceInterface {
    private:
        // D-Bus connection
        std::shared_ptr<sdbus::IConnection> conn;

        // Object path of the device
        sdbus::ObjectPath object_path;

        // Proxy for the device
        std::shared_ptr<sdbus::IProxy> device;

        // Cached characteristic proxies, keyed by UUID
        Characteristics characteristics;



        // Helper to get a characteristic proxy by UUID, with caching
        std::shared_ptr<sdbus::IProxy>& getCharacteristic(std::string_view uuid);

    public:
        BluezDeviceProxy(std::shared_ptr<sdbus::IConnection> conn, sdbus::ObjectPath object_path);
        BluezDeviceProxy(std::shared_ptr<sdbus::IConnection> conn, bluez::BluezDevice device);

        BluezDeviceProxy(const BluezDeviceProxy&) = default;
        BluezDeviceProxy& operator=(const BluezDeviceProxy&) = default;
        BluezDeviceProxy(BluezDeviceProxy&&) noexcept = default;
        BluezDeviceProxy& operator=(BluezDeviceProxy&&) noexcept = default;

        void connect();
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
        std::optional<std::map<std::uint16_t, sdbus::Variant>> getManufacturerData() const override;

        void setAlias(std::string alias) override;

        friend DotMatrix;
    };
}
