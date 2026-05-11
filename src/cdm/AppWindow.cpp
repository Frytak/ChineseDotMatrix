#include "AppWindow.hpp"
#include "QListWidgetItem"
#include "ViewModels/DevicesViewModel.hpp"
#include "qobject.h"
#include "QThread"

AppWindow::AppWindow(QWidget *parent) : QWidget(parent), conn(std::shared_ptr(sdbus::createSystemBusConnection())) {
    devicesViewModel = new DevicesViewModel(conn, this);

    scanButton = new QPushButton("Start BLE Scan", this);
    devicesView = new DevicesView(devicesViewModel, this);
    rotate0Button = new QPushButton("Rotate 0", this);
    rotate1Button = new QPushButton("Rotate 1", this);
    testButton = new QPushButton("Test", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(scanButton);
    layout->addWidget(devicesView);
    layout->addWidget(rotate0Button);
    layout->addWidget(rotate1Button);
    layout->addWidget(testButton);
    setLayout(layout);

    connect(scanButton, &QPushButton::clicked, this, &AppWindow::onScanButtonClicked);
    connect(rotate0Button, &QPushButton::clicked, this, &AppWindow::onRotate0ButtonClicked);
    connect(rotate1Button, &QPushButton::clicked, this, &AppWindow::onRotate1ButtonClicked);
    connect(testButton, &QPushButton::clicked, this, &AppWindow::testButtonClicked);

    setWindowTitle("Chinese DotMatrix");
    resize(350, 500);
}

void AppWindow::onRotate0ButtonClicked() {
    devicesViewModel->connectedDevice()->setRotate180(0);
}

void AppWindow::onRotate1ButtonClicked() {
    devicesViewModel->connectedDevice()->setRotate180(1);
}

void AppWindow::onScanButtonClicked() {
    //scanButton->setEnabled(false);
    devicesViewModel->startScan();
    //scanButton->setEnabled(true);
}

void AppWindow::testButtonClicked() {
    devicesViewModel->connectedDevice()->test();
}
