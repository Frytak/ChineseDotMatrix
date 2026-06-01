#include "ConnectedDeviceView.hpp"
#include "../theme.hpp"
#include "qboxlayout.h"
#include "qnamespace.h"
#include "qpainter.h"
#include "qsvgrenderer.h"
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

ConnectedDeviceView::ConnectedDeviceView(DotMatrixManager* dot_matrix_manager, QWidget *parent) : QWidget(parent), dot_matrix_manager(dot_matrix_manager), previous_name() {
    connected_to_label = new QLabel("Connected to", this);
    connected_to_label->setStyleSheet(QString(R"(
        color: %1;
        font-size: 16px;
    )").arg(QColor::fromRgb(0x80, 0x80, 0x80).name()));

    name_label = new QLabel("", this);
    name_label->setToolTip("");
    name_label->setStyleSheet(QString(R"(
        color: %1;
        font-size: 36px;
    )").arg(Theme::text.name()));

    auto image = QImage(64,64, QImage::Format_ARGB32);
    image.fill(0x00000000);
    auto painter = QPainter(&image);

    disconnect_button = new QPushButton(this);
    disconnect_button->setFixedSize(36, 36);
    disconnect_button->setToolTip("Disconnect");
    QSvgRenderer(QString(":/assets/link-off.svg"), this).render(&painter);
    disconnect_button->setIcon(QIcon(QPixmap::fromImage(image)));
    disconnect_button->setIconSize(QSize(24, 24));

    setVisible(dot_matrix_manager->isConnected());
    QHBoxLayout *layout = new QHBoxLayout(this);

    QVBoxLayout *left_layout = new QVBoxLayout;
    left_layout->addWidget(connected_to_label);
    left_layout->addWidget(name_label);
    left_layout->setAlignment(Qt::AlignBottom | Qt::AlignLeft);

    QVBoxLayout *right_layout = new QVBoxLayout;
    right_layout->addWidget(disconnect_button);
    right_layout->setAlignment(Qt::AlignBottom | Qt::AlignRight);
    right_layout->setContentsMargins(0, 0, 0, 3);

    layout->addLayout(left_layout);
    layout->addLayout(right_layout);
    setLayout(layout);

    connect(disconnect_button, &QPushButton::clicked, this, &ConnectedDeviceView::onDisconnectButtonClicked);

    connect(dot_matrix_manager, &DotMatrixManager::disconnecting, disconnect_button, [this]() { disconnect_button->setDisabled(true); });

    connect(dot_matrix_manager, &DotMatrixManager::nameChanged, name_label, [this](const QString& new_name) {
        if (!previous_name.isEmpty()) { return; }
        name_label->setToolTip(new_name);
        name_label->setText(elideNameLabel(new_name));
    });

    connect(dot_matrix_manager, &DotMatrixManager::connectionChanged, this, [this](bool connected){
        disconnect_button->setDisabled(!connected);
        if (connected) {
            fadeIn();
        } else {
            fadeOut();
        }
    });
}

void ConnectedDeviceView::fadeIn() {
    QParallelAnimationGroup* masterGroup = new QParallelAnimationGroup();

    int delay = 0;

    for (QWidget* widget : {dynamic_cast<QWidget*>(connected_to_label), dynamic_cast<QWidget*>(name_label), dynamic_cast<QWidget*>(disconnect_button)}) {
        // Try to get the existing effect, or create one if it doesn't exist
        QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
        if (!effect) {
            effect = new QGraphicsOpacityEffect(widget);
            widget->setGraphicsEffect(effect);
        }

        effect->setOpacity(0.0); 

        QPropertyAnimation* fade_animation = new QPropertyAnimation(effect, "opacity");
        fade_animation->setDuration(FADE_DURATION);
        fade_animation->setStartValue(0.0);
        fade_animation->setEndValue(1.0);

        QSequentialAnimationGroup* sequence = new QSequentialAnimationGroup();
        if (delay > 0) { sequence->addPause(delay); }
        sequence->addAnimation(fade_animation);

        masterGroup->addAnimation(sequence);

        delay += FADE_INBETWEEN_DELAY; 
    }

    this->show();
    masterGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void ConnectedDeviceView::fadeOut() {
    QParallelAnimationGroup* masterGroup = new QParallelAnimationGroup();

    int delay = 0;

    for (QWidget* widget : {dynamic_cast<QWidget*>(disconnect_button), dynamic_cast<QWidget*>(name_label), dynamic_cast<QWidget*>(connected_to_label)}) {
        // Try to get the existing effect, or create one if it doesn't exist
        QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
        if (!effect) {
            effect = new QGraphicsOpacityEffect(widget);
            widget->setGraphicsEffect(effect);
        }

        effect->setOpacity(1.0); 

        QPropertyAnimation* fade_animation = new QPropertyAnimation(effect, "opacity");
        fade_animation->setDuration(FADE_DURATION);
        fade_animation->setStartValue(1.0);
        fade_animation->setEndValue(0.0);

        QSequentialAnimationGroup* sequence = new QSequentialAnimationGroup();
        if (delay > 0) { sequence->addPause(delay); }
        sequence->addAnimation(fade_animation);

        masterGroup->addAnimation(sequence);

        delay += FADE_INBETWEEN_DELAY; 
    }

    connect(masterGroup, &QParallelAnimationGroup::finished, this, [this]() {
        this->hide();
        previous_name.clear();
    });
    masterGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void ConnectedDeviceView::onDisconnectButtonClicked() {
    emit disconnectButtonClicked();
    previous_name = name_label->text();
    dot_matrix_manager->disconnectFromDevice();
}

QString ConnectedDeviceView::elideNameLabel(const QString& name) {
    QFontMetrics metrics(name_label->font());
    return metrics.elidedText(name, Qt::ElideRight, 250);
}
