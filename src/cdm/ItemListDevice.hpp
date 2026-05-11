#pragma once

#include <QWidget>
#include <sdbus-c++/Types.h>
#include "../lib/BluezDevice.hpp"

class ItemListDevice : public QWidget {
    Q_OBJECT

public:
    explicit ItemListDevice(const bluez::BluezDevice& device, QWidget *parent = nullptr);

private:
    // D-Bus object path of the device
    sdbus::ObjectPath object_path;

    // Human readable name of the device
    QString name;

public:
    const sdbus::ObjectPath& getObjectPath() const;
    const QString& getName() const;
};
