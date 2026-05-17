#pragma once
#include <QObject>
#include <QString>
#include <QThread>
#include <QColor>
#include <optional>
#include <queue>
#include <span>

#include "../lib/DotMatrix.hpp"

class DotMatrixManager : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)

public:
    explicit DotMatrixManager(std::shared_ptr<sdbus::IConnection> conn, QObject *parent = nullptr);

    QString name() const;
    bool isConnected() const;
    std::span<const bluez::BluezDevice> discoveredDevices() const;

public slots:
    Q_INVOKABLE void startScan();
    Q_INVOKABLE void stopScan();

    Q_INVOKABLE void addDevice(bluez::BluezDevice device);
    Q_INVOKABLE void connectToDevice(bluez::BluezDevice device);
    Q_INVOKABLE void disconnectFromDevice();

    Q_INVOKABLE void setRotate180(bool rotate);
    Q_INVOKABLE void test();
    Q_INVOKABLE void setPixel(std::uint8_t x, std::uint8_t y, const QColor& pixel);
    Q_INVOKABLE void setDrawingMode();

signals:
    void nameChanged(const QString& new_name);
    void connectionChanged(bool connected);
    void connectedDeviceChanged(const std::optional<DotMatrix>& connected_dot_matrix);
    void isScanningChanged(bool scanning);
    void deviceDiscovered(const bluez::BluezDevice& device);

private:
    std::shared_ptr<sdbus::IConnection> conn;
    std::optional<DotMatrix> connected_dot_matrix = std::nullopt;
    std::vector<bluez::BluezDevice> devices = {};
    std::queue<std::tuple<std::uint8_t, std::uint8_t, QColor>> draw_queue = {};
    std::mutex draw_queue_mutex;
    std::condition_variable draw_queue_cv;
    QThread* draw_thread = nullptr;
    QThread* scan_thread = nullptr;

    friend class DevicesViewModel;
};
