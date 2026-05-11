#include <QObject>
#include <iostream>
#include <optional>
#include "DevicesViewModel.hpp"
#include "QThread"

DevicesViewModel::DevicesViewModel(std::shared_ptr<sdbus::IConnection> conn, QObject *parent) : QObject(parent), conn(conn) {
    connect(this, &DevicesViewModel::deviceAdded, this, &DevicesViewModel::onDeviceAdded);
};

bool DevicesViewModel::isScanning() const {
    return _scan_thread != nullptr;
}

DeviceViewModel* DevicesViewModel::connectedDevice() const {
    return _connected_device;
}

const QList<DeviceViewModel*>& DevicesViewModel::discoveredDevices() const {
    return _devices;
}

// TODO: handle errors
void DevicesViewModel::startScan() {
    if (_scan_thread) { return; }

    _scan_thread = QThread::create([this]() {
        DotMatrix::scan(conn, [&](bluez::BluezDevice device) {
            QMetaObject::invokeMethod(this, [this, device]() {
                addDevice(device);
            });
        });
    });

    _scan_thread->setParent(this);
    // TODO: handle scan finished
    //connect(_scan_thread, &QThread::finished, _scan_thread, &DevicesViewModel::onScanFinished);
    _scan_thread->start();

    emit isScanningChanged(true);
}

// TODO: handle errors
void DevicesViewModel::stopScan() {
    if (!_scan_thread) { return; }
    emit isScanningChanged(false);
}

void DevicesViewModel::addDevice(bluez::BluezDevice device) {
    addDevice(bluez::BluezDeviceProxy(conn, device));
}

void DevicesViewModel::addDevice(bluez::BluezDeviceProxy device) {
    addDevice(new DeviceViewModel(device, this));
}

void DevicesViewModel::addDevice(DeviceViewModel* device) {
    _devices.append(device);
    emit deviceAdded(device);
}

void DevicesViewModel::onDeviceAdded(DeviceViewModel* device) {
}

// TODO: handle errors
void DevicesViewModel::connectToDevice(DeviceViewModel* device) {
    if (_connected_device) {
        // TODO: disconnect from the currently connected device before connecting to a new one
        throw "Not yet implemented";
    } else {
        device->connectToDevice();
        _connected_device = device;
    }

    emit connectedDeviceChanged(_connected_device);
}

// TODO: handle errors
void DevicesViewModel::disconnectFromDevice() {
    if (!_connected_device) { return; }

    _connected_device->disconnectFromDevice();
    _connected_device = nullptr;

    emit connectedDeviceChanged(_connected_device);
}
