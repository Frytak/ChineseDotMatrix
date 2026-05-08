#include "AppWindow.hpp"
#include <optional>

AppWindow::AppWindow(QWidget *parent) : QWidget(parent), conn(std::shared_ptr(sdbus::createSystemBusConnection())), matrix(std::nullopt) {
    scanButton = new QPushButton("Start BLE Scan", this);
    deviceList = new QListWidget(this);
    rotate0Button = new QPushButton("Rotate 0", this);
    rotate1Button = new QPushButton("Rotate 1", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(scanButton);
    layout->addWidget(deviceList);
    layout->addWidget(rotate0Button);
    layout->addWidget(rotate1Button);
    setLayout(layout);

    connect(scanButton, &QPushButton::clicked, this, &AppWindow::onScanButtonClicked);
    connect(rotate0Button, &QPushButton::clicked, this, &AppWindow::onRotate0ButtonClicked);
    connect(rotate1Button, &QPushButton::clicked, this, &AppWindow::onRotate1ButtonClicked);

    setWindowTitle("Chinese DotMatrix");
    resize(350, 500);
}

void AppWindow::onRotate0ButtonClicked() {
    matrix->setRotate180(0);
}

void AppWindow::onRotate1ButtonClicked() {
    matrix->setRotate180(1);
}

void AppWindow::onScanButtonClicked() {
    scanButton->setEnabled(false);
    deviceList->clear();
    deviceList->addItem("Scanning...");

    auto devices = DotMatrix::scan(conn);
    matrix = DotMatrix(conn, devices.begin()->first);
    
    deviceList->clear();
    for (const auto& [path, interfaces] : devices) {
        if (interfaces.contains(DEVICE_IFACE)) {
            const auto& properties = interfaces.at(DEVICE_IFACE);
            if (properties.contains("Name")) {
                QString name = QString::fromStdString(properties.at("Name").get<std::string>());
                onDeviceFound(name);
            }
        }
    }
    
    scanButton->setEnabled(true);
}

void AppWindow::onDeviceFound(const QString& deviceName) {
    deviceList->addItem(deviceName);
}
