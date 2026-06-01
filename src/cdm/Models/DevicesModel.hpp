#pragma once
#include <QObject>
#include <QAbstractItemModel>

#include "../DotMatrixManager.hpp"
#include "qvariant.h"

class DevicesModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum DeviceRoles {
        IsConnectedRole = Qt::UserRole + 1
    };

    explicit DevicesModel(DotMatrixManager* manager, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    void setConnectedDevice(int row);

private slots:
    void onDeviceDiscovered(const bluez::BluezDevice& device);
    void onDiscoveredDevicesCleared();

private:
    DotMatrixManager* manager;
    int connectedDeviceRow = -1;

    friend class DevicesView;
};
