#pragma once
#include <QObject>
#include "DeviceViewModel.hpp"

class DevicesViewModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isScanning READ isScanning NOTIFY isScanningChanged)
    Q_PROPERTY(DeviceViewModel* connectedDevice READ connectedDevice NOTIFY connectedDeviceChanged)

public:
    explicit DevicesViewModel(std::shared_ptr<sdbus::IConnection> conn, QObject *parent = nullptr);

    bool isScanning() const;
    DeviceViewModel* connectedDevice() const;
    const QList<DeviceViewModel*>& discoveredDevices() const;

public slots:
    Q_INVOKABLE void startScan();
    Q_INVOKABLE void stopScan();
    Q_INVOKABLE void addDevice(bluez::BluezDevice device);
    Q_INVOKABLE void addDevice(bluez::BluezDeviceProxy device);
    Q_INVOKABLE void addDevice(DeviceViewModel* device);
    Q_INVOKABLE void connectToDevice(DeviceViewModel* device);
    Q_INVOKABLE void disconnectFromDevice();

    void onDeviceAdded(DeviceViewModel* device);

signals:
    void isScanningChanged(bool scanning);
    void connectedDeviceChanged(DeviceViewModel* device);
    void deviceAdded(DeviceViewModel* device);

private:
    // D-Bus connection
    std::shared_ptr<sdbus::IConnection> conn;

    DeviceViewModel* _connected_device = nullptr;

    QThread* _scan_thread = nullptr;
    
    QList<DeviceViewModel*> _devices;
};
