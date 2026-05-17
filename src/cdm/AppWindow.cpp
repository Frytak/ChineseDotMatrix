#include "AppWindow.hpp"
#include "QListWidgetItem"
#include "Models/DevicesModel.hpp"
#include "qobject.h"
#include "QThread"
#include <QColorDialog>

AppWindow::AppWindow(QWidget *parent) : QWidget(parent), conn(std::shared_ptr(sdbus::createSystemBusConnection())) {
    dot_matrix_manager = new DotMatrixManager(conn, this);
    devicesModel = new DevicesModel(dot_matrix_manager, this);
    pixelGridModel = new PixelGridModel(this);

    scanButton = new QPushButton("Start BLE Scan", this);
    rotate0Button = new QPushButton("Rotate 0°", this);
    rotate1Button = new QPushButton("Rotate 180°", this);
    testButton = new QPushButton("Test", this);
    colorButton = new QPushButton("Color", this);
    colorButton->setFixedSize(36, 36);
    clearButton = new QPushButton("Clear", this);

    devicesView = new DevicesView(devicesModel, this);
    pixelGridView = new PixelGridView(pixelGridModel, this);

    updateColorButton(pixelGridModel->currentColor());

    auto* toolbar = new QHBoxLayout;
    toolbar->addWidget(colorButton);
    toolbar->addWidget(clearButton);
    toolbar->addStretch();
    toolbar->addWidget(rotate0Button);
    toolbar->addWidget(rotate1Button);
    toolbar->addWidget(testButton);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(scanButton);
    layout->addWidget(devicesView);
    layout->addLayout(toolbar);
    layout->addWidget(pixelGridView);
    setLayout(layout);

    connect(scanButton, &QPushButton::clicked, this, &AppWindow::onScanButtonClicked);
    connect(rotate0Button, &QPushButton::clicked, this, &AppWindow::onRotate0ButtonClicked);
    connect(rotate1Button, &QPushButton::clicked, this, &AppWindow::onRotate1ButtonClicked);
    connect(testButton, &QPushButton::clicked, this, &AppWindow::testButtonClicked);

    connect(colorButton,   &QPushButton::clicked, this, &AppWindow::onPickColorButtonClicked);
    connect(clearButton,   &QPushButton::clicked, this, &AppWindow::onClearButtonClicked);

    connect(pixelGridModel, &PixelGridModel::pixelChanged, dot_matrix_manager, &DotMatrixManager::setPixel);
    connect(pixelGridModel, &PixelGridModel::cleared, dot_matrix_manager, &DotMatrixManager::setDrawingMode);

    setWindowTitle("Chinese DotMatrix");
    resize(350, 500);
}

void AppWindow::onRotate0ButtonClicked() { dot_matrix_manager->setRotate180(0); }
void AppWindow::onRotate1ButtonClicked() { dot_matrix_manager->setRotate180(1); }

void AppWindow::onScanButtonClicked() {
    //scanButton->setEnabled(false);
    dot_matrix_manager->startScan();
    //scanButton->setEnabled(true);
}

void AppWindow::testButtonClicked() { dot_matrix_manager->test(); }

void AppWindow::onPickColorButtonClicked() {
    QColor picked = QColorDialog::getColor(pixelGridModel->currentColor(), this, "Paint color");
    if (!picked.isValid()) return;
    pixelGridModel->setCurrentColor(picked);
    updateColorButton(picked);
}

void AppWindow::onClearButtonClicked() {
    pixelGridModel->clear();
}

void AppWindow::updateColorButton(const QColor& c) {
    colorButton->setStyleSheet(QString("background-color: %1; border: 2px solid #555;").arg(c.name()));
}
