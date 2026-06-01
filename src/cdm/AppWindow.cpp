#include "AppWindow.hpp"
#include "Views/ConnectedDeviceView.hpp"
#include "LayeredSvgDisplay.hpp"
#include "QListWidgetItem"
#include "Models/DevicesModel.hpp"
#include "qboxlayout.h"
#include "qgridlayout.h"
#include "qnamespace.h"
#include "theme.hpp"
#include <QBoxLayout>
#include <QSvgRenderer>
#include <QObject>
#include <QThread>
#include <QLabel>
#include <QSvgWidget>
#include <QIconEngine>
#include <QColorDialog>
#include "../lib/DBusConnectionManager.hpp"

AppWindow::AppWindow(QWidget *parent) : QWidget(parent), conn(DBusConnectionManager::getSystemBus()) {
    dot_matrix_manager = new DotMatrixManager(conn, this);
    devices_model = new DevicesModel(dot_matrix_manager, this);
    pixel_grid_model = new PixelGridModel(this);

    auto image = QImage(64,64, QImage::Format_ARGB32);
    image.fill(0x00000000);
    auto painter = QPainter(&image);

    scan_button = new QPushButton("Start BLE Scan", this);

    rotate0_button = new QPushButton(this);
    rotate0_button->setFixedSize(36, 36);
    rotate0_button->setToolTip("Right side down");
    QSvgRenderer(QString(":/assets/rotate0.svg"), this).render(&painter);
    rotate0_button->setIcon(QIcon(QPixmap::fromImage(image)));
    rotate0_button->setIconSize(QSize(24, 24));
    image.fill(0x00000000);

    rotate1_button = new QPushButton(this);
    rotate1_button->setFixedSize(36, 36);
    rotate1_button->setToolTip("Upside down");
    QSvgRenderer(QString(":/assets/rotate180.svg"), this).render(&painter);
    rotate1_button->setIcon(QIcon(QPixmap::fromImage(image)));
    rotate1_button->setIconSize(QSize(24, 24));
    image.fill(0x00000000);

    clear_button = new QPushButton(this);
    clear_button->setFixedSize(36, 36);
    clear_button->setToolTip("Clear");
    QSvgRenderer(QString(":/assets/delete.svg"), this).render(&painter);
    clear_button->setIcon(QIcon(QPixmap::fromImage(image)));
    clear_button->setIconSize(QSize(24, 24));

    color_button = new QPushButton("Color", this);
    color_button->setFixedSize(36, 36);

    devices_view = new DevicesView(devices_model, this);
    pixel_grid_view = new PixelGridView(pixel_grid_model, this);
    pixel_grid_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    updateColorButton(pixel_grid_model->primaryColor());

    auto* connectionArea = new QWidget;
    connectionArea->setFixedWidth(300);
    connectionArea->setFixedHeight(420);
    connectionArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    auto* connectionAreaLayout = new QVBoxLayout(connectionArea);
    connectionAreaLayout->setContentsMargins(20, 20, 20, 20);
    connectionAreaLayout->addWidget(scan_button);
    connectionAreaLayout->addWidget(devices_view);

    auto* toolbarArea = new QWidget;
    toolbarArea->setFixedWidth(90);
    toolbarArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    auto* toolbarAreaLayout = new QVBoxLayout(toolbarArea);
    toolbarAreaLayout->setAlignment(Qt::AlignHCenter);
    toolbarAreaLayout->setContentsMargins(0, 0, 0, 0);
    toolbarAreaLayout->addWidget(color_button);
    toolbarAreaLayout->addWidget(clear_button);
    toolbarAreaLayout->addWidget(rotate0_button);
    toolbarAreaLayout->addWidget(rotate1_button);
    toolbarAreaLayout->addStretch();

    // Devices background
    LayeredSvgDisplay *devicesBackground = new LayeredSvgDisplay(":/assets/devicesBackground.svg", this);
    devicesBackground->setFixedWidth(550);
    devicesBackground->setMinimumHeight(550);
    devicesBackground->setAttribute(Qt::WA_TransparentForMouseEvents);

    std::vector<LayeredSvgDisplay::Layer> devicesBackgroundSvgs = {
        { Theme::background.darker(140), QPointF(-100, 0), 1.00 },
        { Theme::background.darker(180), QPointF(-120, -20), 0.95 },
        { Theme::background.darker(220), QPointF(-140, -40), 0.90 }
    };
    devicesBackground->setLayers(devicesBackgroundSvgs);

    // Connected device background
    LayeredSvgDisplay *connectedDeviceBackground = new LayeredSvgDisplay(":/assets/connectedDeviceBackground.svg", this);
    connectedDeviceBackground->setFixedWidth(550);
    connectedDeviceBackground->setMinimumHeight(550);
    connectedDeviceBackground->setAttribute(Qt::WA_TransparentForMouseEvents);

    std::vector<LayeredSvgDisplay::Layer> connectedDeviceBackgroundSvgs = {
        { Theme::background.darker(140), QPointF(0, 120), 1.00 },
        { Theme::background.darker(180), QPointF(-20, 140), 0.95 },
        { Theme::background.darker(220), QPointF(-40, 160), 0.90 }
    };
    connectedDeviceBackground->setLayers(connectedDeviceBackgroundSvgs);

    auto* connectedDevice = new ConnectedDeviceView(dot_matrix_manager, this);
    connectedDevice->setFixedWidth(400);
    connectedDevice->setContentsMargins(20, 0, 0, 50);
    connectedDevice->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connectedDevice->setVisible(false);

    // Toolbar background
    LayeredSvgDisplay *toolbarBackground = new LayeredSvgDisplay(":/assets/toolsBackground.svg", this);
    toolbarBackground->setFixedWidth(580);
    toolbarBackground->setMinimumHeight(1080);
    toolbarBackground->setAttribute(Qt::WA_TransparentForMouseEvents);

    std::vector<LayeredSvgDisplay::Layer> toolbarBackgroundSvgs = {
        { Theme::background.darker(140), QPointF(0, 0), 1.20 },
        { Theme::background.darker(180), QPointF(30, 15), 1.20 },
        { Theme::background.darker(220), QPointF(60, 30), 1.20 }
    };
    toolbarBackground->setLayers(toolbarBackgroundSvgs);

    auto* layout = new QGridLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 0);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(1, 1);

    layout->addWidget(devicesBackground, 0, 0, Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(toolbarBackground, 0, 2, Qt::AlignRight | Qt::AlignBottom);
    layout->addWidget(connectedDeviceBackground, 1, 0, Qt::AlignLeft | Qt::AlignBottom);
    layout->addWidget(connectedDevice, 1, 0, Qt::AlignLeft | Qt::AlignBottom);
    layout->addWidget(connectionArea, 0, 0, Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(pixel_grid_view, 0, 1, 2, 1);
    layout->addWidget(toolbarArea, 0, 2, 2, 1, Qt::AlignRight | Qt::AlignVCenter);
    setLayout(layout);

    connect(scan_button, &QPushButton::clicked, this, &AppWindow::onScanButtonClicked);
    connect(dot_matrix_manager, &DotMatrixManager::isScanningChanged, this, [this](bool scanning) {
        scan_button->setEnabled(!scanning);
    });
    connect(rotate0_button, &QPushButton::clicked, this, &AppWindow::onRotate0ButtonClicked);
    connect(rotate1_button, &QPushButton::clicked, this, &AppWindow::onRotate1ButtonClicked);

    connect(color_button,   &QPushButton::clicked, this, &AppWindow::onPickColorButtonClicked);
    connect(clear_button,   &QPushButton::clicked, this, &AppWindow::onClearButtonClicked);

    connect(pixel_grid_model, &PixelGridModel::pixelChanged, dot_matrix_manager, &DotMatrixManager::setPixel);
    connect(pixel_grid_model, &PixelGridModel::cleared, dot_matrix_manager, &DotMatrixManager::setDrawingMode);

    setWindowTitle("Chinese DotMatrix");
    resize(350, 500);
    onScanButtonClicked();
}

void AppWindow::onRotate0ButtonClicked() { dot_matrix_manager->setRotate180(0); }
void AppWindow::onRotate1ButtonClicked() { dot_matrix_manager->setRotate180(1); }

void AppWindow::onScanButtonClicked() {
    dot_matrix_manager->clearDiscoveredDevices();
    dot_matrix_manager->startScan();
}

void AppWindow::onPickColorButtonClicked() {
    QColor picked = QColorDialog::getColor(pixel_grid_model->primaryColor(), this, "Paint color");
    if (!picked.isValid()) return;
    pixel_grid_model->setPrimaryColor(picked);
    updateColorButton(picked);
}

void AppWindow::onClearButtonClicked() {
    pixel_grid_model->clear();
}

void AppWindow::updateColorButton(const QColor& c) {
    color_button->setStyleSheet(QString("background-color: %1; border: 2px solid #555;").arg(c.name()));
}
