#include "DotMatrixManager.hpp"
#include "qthread.h"
#include <QColor>
#include <optional>
#include <queue>
#include <tuple>

DotMatrixManager::DotMatrixManager(std::shared_ptr<sdbus::IConnection> conn, QObject *parent)
    : QObject(parent)
    , conn(conn)
{
    connect(this, &DotMatrixManager::connectedDeviceChanged, this, [this](const std::optional<DotMatrix>& connected_dot_matrix) {
        emit nameChanged(connected_dot_matrix ? QString::fromStdString(connected_dot_matrix->getAlias()) : QString());
        emit connectionChanged(connected_dot_matrix.has_value());
    });

    draw_thread = QThread::create([this]() {
        while (true) {
            std::unique_lock<std::mutex> lock(draw_queue_mutex);
            draw_queue_cv.wait(lock, [this]() { return !draw_queue.empty(); });

            while (!draw_queue.empty()) {
                const auto [x, y, color] = draw_queue.front();
                draw_queue.pop();
                lock.unlock();

                if (connected_dot_matrix.has_value()) {
                    connected_dot_matrix->setPixel(x, y, {
                        static_cast<std::uint8_t>(color.red()),
                        static_cast<std::uint8_t>(color.green()),
                        static_cast<std::uint8_t>(color.blue())
                    });
                }

                lock.lock();
            }
        }
    });
    draw_thread->setParent(this);
    draw_thread->start();
}

QString DotMatrixManager::name() const {
    if (connected_dot_matrix) {
        return QString::fromStdString(connected_dot_matrix->getAlias());
    }

    throw std::runtime_error("No device connected");
}

bool DotMatrixManager::isConnected() const {
    return connected_dot_matrix.has_value();
}

std::span<const bluez::BluezDevice> DotMatrixManager::discoveredDevices() const {
    return devices;
}

void DotMatrixManager::startScan() {
    if (scan_thread) { return; }

    scan_thread = QThread::create([this]() {
        DotMatrix::scan(conn, [&](bluez::BluezDevice device) {
            QMetaObject::invokeMethod(this, [this, device]() {
                addDevice(device);
            });
        });

        QMetaObject::invokeMethod(this, [this]() {
            emit isScanningChanged(false);
        });
    });

    scan_thread->setParent(this);
    // TODO: handle scan finished
    //connect(_scan_thread, &QThread::finished, _scan_thread, &DevicesViewModel::onScanFinished);
    scan_thread->start();

    emit isScanningChanged(true);
}

void DotMatrixManager::stopScan() {
    if (!scan_thread) { return; }
    emit isScanningChanged(false);
}

void DotMatrixManager::addDevice(bluez::BluezDevice device) {
    devices.push_back(device);
    emit deviceDiscovered(device);
}

void DotMatrixManager::connectToDevice(bluez::BluezDevice device) {
    if (connected_dot_matrix.has_value()) { return; }

    QThread::create([this, device]() {
        DotMatrix dot_matrix(conn, device);
        dot_matrix.connect();

        QMetaObject::invokeMethod(this, [this, dot_matrix]() {
            connected_dot_matrix = dot_matrix;
            emit connectedDeviceChanged(connected_dot_matrix);
        });
    })->start();
}

void DotMatrixManager::disconnectFromDevice() {
    if (!connected_dot_matrix.has_value()) { return; }

    QThread::create([this]() {
        connected_dot_matrix->disconnect();

        QMetaObject::invokeMethod(this, [this]() {
            connected_dot_matrix = std::nullopt;
            emit connectedDeviceChanged(connected_dot_matrix);
        });
    })->start();
}

void DotMatrixManager::setRotate180(bool rotate) {
    if (!connected_dot_matrix.has_value()) { return; }
        
    connected_dot_matrix->setRotate180(rotate);
}

void DotMatrixManager::setPixel(std::uint8_t x, std::uint8_t y, const QColor& color) {
    if (!connected_dot_matrix.has_value()) { return; }

    {
        std::lock_guard<std::mutex> lock(draw_queue_mutex);
        draw_queue.push({x, y, color});
    }

    draw_queue_cv.notify_one();
}

void DotMatrixManager::setDrawingMode() {
    if (!connected_dot_matrix.has_value()) { return; }
    connected_dot_matrix->setDrawingMode();
}
