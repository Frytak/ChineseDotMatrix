#ifndef LIB_H
#define LIB_H

#include <functional>
#include <sdbus-c++/Types.h>
#include <sdbus-c++/sdbus-c++.h>

extern "C" {

constexpr auto BLUEZ_SERVICE        = "org.bluez";
constexpr auto ADAPTER_IFACE        = "org.bluez.Adapter1";
constexpr auto DEVICE_IFACE         = "org.bluez.Device1";
constexpr auto GATT_CHAR_IFACE      = "org.bluez.GattCharacteristic1";
constexpr auto OBJECT_MANAGER_IFACE = "org.freedesktop.DBus.ObjectManager";
constexpr auto PROPERTIES_IFACE     = "org.freedesktop.DBus.Properties";

constexpr auto UUID_WRITE_CHAR = "0000fa02-0000-1000-8000-00805f9b34fb";
constexpr auto UUID_READ_CHAR  = "00002902-0000-1000-8000-00805f9b34fb";

// Objects returned by GetManagedObjects
using ManagedObjects = std::map<
    sdbus::ObjectPath,
    std::map<std::string, std::map<std::string, sdbus::Variant>>
>;

class DotMatrix {
private:
    // D-Bus connection
    std::shared_ptr<sdbus::IConnection> conn;

    // Proxy for the device
    std::unique_ptr<sdbus::IProxy> device;

    // Object path of the device
    sdbus::ObjectPath device_path;

    // Cached characteristic proxies, keyed by UUID
    std::map<std::string, std::unique_ptr<sdbus::IProxy>, std::less<>> characteristics;



    // Helper to get a characteristic proxy by UUID, with caching
    std::unique_ptr<sdbus::IProxy>& getCharacteristic(std::string_view uuid);

    // Write data to a characteristic by UUID
    void write(std::string_view uuid, std::vector<uint8_t> data);

    // Read data from a characteristic by UUID
    std::vector<uint8_t> read(std::string_view uuid);

public:
    DotMatrix(std::shared_ptr<sdbus::IConnection> conn, sdbus::ObjectPath device_path);

    // Scan for DotMatrix devices
    static ManagedObjects scan(std::shared_ptr<sdbus::IConnection> conn);

    void setRotate180(bool rotate);
};

}

#endif
