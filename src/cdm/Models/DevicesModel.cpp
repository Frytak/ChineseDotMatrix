#include <QObject>
#include "DevicesModel.hpp"
#include "QThread"


DevicesModel::DevicesModel(DotMatrixManager* manager, QObject *parent) : QAbstractListModel(parent), manager(manager) {
    connect(manager, &DotMatrixManager::deviceDiscovered, this, &DevicesModel::onDeviceDiscovered);
    connect(manager, &DotMatrixManager::discoveredDevicesCleared, this, &DevicesModel::onDiscoveredDevicesCleared);
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

void DevicesModel::onDiscoveredDevicesCleared() {
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
