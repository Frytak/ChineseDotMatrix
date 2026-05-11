#pragma once
#include <QObject>
#include <QString>
#include "../../lib/DotMatrix.hpp"
#include "qtmetamacros.h"

class DeviceViewModel : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)

public:
    explicit DeviceViewModel(bluez::BluezDeviceProxy device, QObject *parent = nullptr);

    QString name() const;    
    bool isConnected() const;

public slots:
    Q_INVOKABLE void setRotate180(bool rotate);
    Q_INVOKABLE void test();

signals:
    void nameChanged(const QString& new_name);
    void connectionChanged(bool connected);

private:
    bluez::BluezDeviceProxy device;
    std::optional<DotMatrix*> dot_matrix;

    Q_INVOKABLE void connectToDevice();
    Q_INVOKABLE void disconnectFromDevice();

    friend class DevicesViewModel;
};
