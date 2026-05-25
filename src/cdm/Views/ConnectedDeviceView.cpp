#include "ConnectedDeviceView.hpp"
#include "../theme.hpp"
#include "qboxlayout.h"
#include "qpainter.h"

ConnectedDeviceView::ConnectedDeviceView(DotMatrixManager* dot_matrix_manager, QWidget *parent) : QWidget(parent), dot_matrix_manager(dot_matrix_manager) {
    QLabel* connected_to_label = new QLabel("Connected to", this);
    connected_to_label->setStyleSheet(QString(R"(
        color: %1;
        font-size: 16px;
    )").arg(QColor::fromRgb(0x80, 0x80, 0x80).name()));

    name_label = new QLabel("", this);
    name_label->setToolTip("");
    name_label->setFixedWidth(250);
    name_label->setStyleSheet(QString(R"(
        color: %1;
        font-size: 36px;
    )").arg(Theme::text.name()));

    disconnect_button = new QPushButton("Disconnect", this);
    auto image = QImage(64,64, QImage::Format_ARGB32);
    image.fill(0x00000000);
    auto painter = QPainter(&image);

    setVisible(dot_matrix_manager->isConnected());
    QHBoxLayout *layout = new QHBoxLayout(this);

    QVBoxLayout *left_layout = new QVBoxLayout;
    left_layout->addWidget(connected_to_label);
    left_layout->addWidget(name_label);
    left_layout->addStretch();

    QVBoxLayout *right_layout = new QVBoxLayout;
    right_layout->addWidget(disconnect_button);

    layout->addLayout(left_layout);
    layout->addLayout(right_layout);
    setLayout(layout);

    connect(disconnect_button, &QPushButton::clicked, this, &ConnectedDeviceView::onDisconnectButtonClicked);

    connect(dot_matrix_manager, &DotMatrixManager::nameChanged, name_label, [this](const QString& new_name) {
        name_label->setToolTip(new_name);
        name_label->setText(elideNameLabel(new_name));
    });

    connect(dot_matrix_manager, &DotMatrixManager::connectionChanged, this, [this](bool connected){
        this->setVisible(connected);
    });
}

void ConnectedDeviceView::onDisconnectButtonClicked() {
    dot_matrix_manager->disconnectFromDevice();
    emit disconnectButtonClicked();
}

QString ConnectedDeviceView::elideNameLabel(const QString& name) {
    QFontMetrics metrics(name_label->font());
    return metrics.elidedText(name, Qt::ElideRight, 250);
}
