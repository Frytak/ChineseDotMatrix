#include "DeviceViewModel.hpp"
#include <optional>

DeviceViewModel::DeviceViewModel(bluez::BluezDeviceProxy device, QObject *parent) : QObject(parent), device(device) {}

QString DeviceViewModel::name() const { 
    return QString::fromStdString(device.getAlias()); 
}

bool DeviceViewModel::isConnected() const { 
    return dot_matrix.has_value(); 
}

void DeviceViewModel::setRotate180(bool rotate) {
    if (!dot_matrix.has_value()) { return; }
    dot_matrix.value()->setRotate180(rotate);
}

void DeviceViewModel::test() {
    if (!dot_matrix.has_value()) { return; }
    dot_matrix.value()->test();
}

void DeviceViewModel::connectToDevice() {
    dot_matrix = new DotMatrix(device);
    emit connectionChanged(true);
}

void DeviceViewModel::disconnectFromDevice() {
    throw "Not yet implemented";
}
