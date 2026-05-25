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

AppWindow::AppWindow(QWidget *parent) : QWidget(parent), conn(std::shared_ptr(sdbus::createSystemBusConnection())) {
    dot_matrix_manager = new DotMatrixManager(conn, this);
    devicesModel = new DevicesModel(dot_matrix_manager, this);
    pixelGridModel = new PixelGridModel(this);

    auto image = QImage(64,64, QImage::Format_ARGB32);
    image.fill(0x00000000);
    auto painter = QPainter(&image);

    scanButton = new QPushButton("Start BLE Scan", this);

    rotate0Button = new QPushButton(this);
    rotate0Button->setFixedSize(36, 36);
    rotate0Button->setToolTip("Right side down");
    QSvgRenderer(QString(":/assets/rotate0.svg"), this).render(&painter);
    rotate0Button->setIcon(QIcon(QPixmap::fromImage(image)));
    rotate0Button->setIconSize(QSize(24, 24));
    image.fill(0x00000000);

    rotate1Button = new QPushButton(this);
    rotate1Button->setFixedSize(36, 36);
    rotate1Button->setToolTip("Upside down");
    QSvgRenderer(QString(":/assets/rotate180.svg"), this).render(&painter);
    rotate1Button->setIcon(QIcon(QPixmap::fromImage(image)));
    rotate1Button->setIconSize(QSize(24, 24));
    image.fill(0x00000000);

    clearButton = new QPushButton(this);
    clearButton->setFixedSize(36, 36);
    clearButton->setToolTip("Clear");
    QSvgRenderer(QString(":/assets/delete.svg"), this).render(&painter);
    clearButton->setIcon(QIcon(QPixmap::fromImage(image)));
    clearButton->setIconSize(QSize(24, 24));

    colorButton = new QPushButton("Color", this);
    colorButton->setFixedSize(36, 36);

    devicesView = new DevicesView(devicesModel, this);
    pixelGridView = new PixelGridView(pixelGridModel, this);
    pixelGridView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    updateColorButton(pixelGridModel->primaryColor());

    auto* connectionArea = new QWidget;
    connectionArea->setFixedWidth(300);
    connectionArea->setFixedHeight(420);
    connectionArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    auto* connectionAreaLayout = new QVBoxLayout(connectionArea);
    connectionAreaLayout->setContentsMargins(20, 20, 20, 20);
    connectionAreaLayout->addWidget(scanButton);
    connectionAreaLayout->addWidget(devicesView);

    auto* toolbarArea = new QWidget;
    toolbarArea->setFixedWidth(90);
    toolbarArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    auto* toolbarAreaLayout = new QVBoxLayout(toolbarArea);
    toolbarAreaLayout->setAlignment(Qt::AlignHCenter);
    toolbarAreaLayout->setContentsMargins(0, 0, 0, 0);
    toolbarAreaLayout->addWidget(colorButton);
    toolbarAreaLayout->addWidget(clearButton);
    toolbarAreaLayout->addWidget(rotate0Button);
    toolbarAreaLayout->addWidget(rotate1Button);
    toolbarAreaLayout->addStretch();

    // Devices background
    LayeredSvgDisplay *devicesBackground = new LayeredSvgDisplay(":/assets/devicesBackground.svg", this);
    devicesBackground->setFixedWidth(550);
    devicesBackground->setMinimumHeight(550);

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

    std::vector<LayeredSvgDisplay::Layer> connectedDeviceBackgroundSvgs = {
        { Theme::background.darker(140), QPointF(0, 120), 1.00 },
        { Theme::background.darker(180), QPointF(-20, 140), 0.95 },
        { Theme::background.darker(220), QPointF(-40, 160), 0.90 }
    };
    connectedDeviceBackground->setLayers(connectedDeviceBackgroundSvgs);

    auto* connectedDevice = new ConnectedDeviceView(dot_matrix_manager, this);
    connectedDevice->setFixedWidth(350);
    connectedDevice->setFixedHeight(130);
    connectedDevice->setContentsMargins(20, 0, 0, 0);
    connectedDevice->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connectedDevice->setVisible(false);

    // Toolbar background
    LayeredSvgDisplay *toolbarBackground = new LayeredSvgDisplay(":/assets/toolsBackground.svg", this);
    toolbarBackground->setFixedWidth(580);
    toolbarBackground->setMinimumHeight(1080);

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
    layout->addWidget(pixelGridView, 0, 1, 2, 1);
    layout->addWidget(toolbarArea, 0, 2, 2, 1, Qt::AlignRight | Qt::AlignVCenter);
    setLayout(layout);

    connect(scanButton, &QPushButton::clicked, this, &AppWindow::onScanButtonClicked);
    connect(dot_matrix_manager, &DotMatrixManager::isScanningChanged, this, [this](bool scanning) {
        scanButton->setEnabled(!scanning);
    });
    connect(rotate0Button, &QPushButton::clicked, this, &AppWindow::onRotate0ButtonClicked);
    connect(rotate1Button, &QPushButton::clicked, this, &AppWindow::onRotate1ButtonClicked);

    connect(colorButton,   &QPushButton::clicked, this, &AppWindow::onPickColorButtonClicked);
    connect(clearButton,   &QPushButton::clicked, this, &AppWindow::onClearButtonClicked);

    connect(pixelGridModel, &PixelGridModel::pixelChanged, dot_matrix_manager, &DotMatrixManager::setPixel);
    connect(pixelGridModel, &PixelGridModel::cleared, dot_matrix_manager, &DotMatrixManager::setDrawingMode);

    setWindowTitle("Chinese DotMatrix");
    resize(350, 500);
    onScanButtonClicked();
}

void AppWindow::onRotate0ButtonClicked() { dot_matrix_manager->setRotate180(0); }
void AppWindow::onRotate1ButtonClicked() { dot_matrix_manager->setRotate180(1); }

void AppWindow::onScanButtonClicked() {
    dot_matrix_manager->startScan();
}

void AppWindow::onPickColorButtonClicked() {
    QColor picked = QColorDialog::getColor(pixelGridModel->primaryColor(), this, "Paint color");
    if (!picked.isValid()) return;
    pixelGridModel->setPrimaryColor(picked);
    updateColorButton(picked);
}

void AppWindow::onClearButtonClicked() {
    pixelGridModel->clear();
}

void AppWindow::updateColorButton(const QColor& c) {
    colorButton->setStyleSheet(QString("background-color: %1; border: 2px solid #555;").arg(c.name()));
}
