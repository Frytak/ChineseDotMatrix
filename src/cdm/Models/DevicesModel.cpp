#include <QObject>
#include "DevicesModel.hpp"
#include "QThread"


DevicesModel::DevicesModel(DotMatrixManager* manager, QObject *parent) : QAbstractListModel(parent), manager(manager) {
    connect(manager, &DotMatrixManager::deviceDiscovered, this, &DevicesModel::onDeviceDiscovered);
}

int DevicesModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(manager->discoveredDevices().size());
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) { return QVariant(); }

    auto devices = manager->discoveredDevices();
    const auto &device = devices[static_cast<std::size_t>(index.row())];

    switch (role) {
        case Qt::DisplayRole:
            return QString::fromStdString(device.getAlias());
        case IsConnectedRole:
            return index.row() == connectedDeviceRow;
    }

    return QVariant();
}


void DevicesModel::onDeviceDiscovered(const bluez::BluezDevice&) {
    const int newRow = static_cast<int>(manager->discoveredDevices().size()) - 1;
    beginInsertRows(QModelIndex(), newRow, newRow);
    endInsertRows();
}

void DevicesModel::onDevicesReset() {
    beginResetModel();
    endResetModel();
}

void DevicesModel::setConnectedDevice(int row) {
    int oldConnectedDeviceRow = connectedDeviceRow;
    connectedDeviceRow = row;

    if (oldConnectedDeviceRow >= 0) {
        emit dataChanged(index(oldConnectedDeviceRow), index(oldConnectedDeviceRow), {IsConnectedRole});
    }

    if (connectedDeviceRow >= 0) {
        emit dataChanged(index(connectedDeviceRow), index(connectedDeviceRow), {IsConnectedRole});
    }
}

//DevicesViewModel::DevicesViewModel(std::shared_ptr<sdbus::IConnection> conn, QObject *parent) : QObject(parent), conn(conn) {
//    connect(this, &DevicesViewModel::deviceAdded, this, &DevicesViewModel::onDeviceAdded);
//};
//
//bool DevicesViewModel::isScanning() const {
//    return _scan_thread != nullptr;
//}
//
//DeviceViewModel* DevicesViewModel::connectedDevice() const {
//    return _connected_device;
//}
//
//const QList<DeviceViewModel*>& DevicesViewModel::discoveredDevices() const {
//    return _devices;
//}
//
//// TODO: handle errors
//void DevicesViewModel::startScan() {
//    if (_scan_thread) { return; }
//
//    _scan_thread = QThread::create([this]() {
//        DotMatrix::scan(conn, [&](bluez::BluezDevice device) {
//            QMetaObject::invokeMethod(this, [this, device]() {
//                addDevice(device);
//            });
//        });
//    });
//
//    _scan_thread->setParent(this);
//    // TODO: handle scan finished
//    //connect(_scan_thread, &QThread::finished, _scan_thread, &DevicesViewModel::onScanFinished);
//    _scan_thread->start();
//
//    emit isScanningChanged(true);
//}
//
//// TODO: handle errors
//void DevicesViewModel::stopScan() {
//    if (!_scan_thread) { return; }
//    emit isScanningChanged(false);
//}
//
//void DevicesViewModel::addDevice(bluez::BluezDevice device) {
//    addDevice(bluez::BluezDeviceProxy(conn, device));
//}
//
//void DevicesViewModel::addDevice(bluez::BluezDeviceProxy device) {
//    addDevice(new DeviceViewModel(device, this));
//}
//
//void DevicesViewModel::addDevice(DeviceViewModel* device) {
//    _devices.append(device);
//    emit deviceAdded(device);
//}
//
//void DevicesViewModel::onDeviceAdded(DeviceViewModel*) {
//}
//
//// TODO: handle errors
//void DevicesViewModel::connectToDevice(DeviceViewModel* device) {
//    if (_connected_device) {
//        // TODO: disconnect from the currently connected device before connecting to a new one
//        throw "Not yet implemented";
//    } else {
//        device->connectToDevice();
//        _connected_device = device;
//    }
//
//    emit connectedDeviceChanged(_connected_device);
//}
//
//// TODO: handle errors
//void DevicesViewModel::disconnectFromDevice() {
//    if (!_connected_device) { return; }
//
//    _connected_device->disconnectFromDevice();
//    _connected_device = nullptr;
//
//    emit connectedDeviceChanged(_connected_device);
//}
