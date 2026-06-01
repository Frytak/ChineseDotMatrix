#pragma once

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "../DotMatrixManager.hpp"
#include "qobject.h"
#include "qpushbutton.h"

class ConnectedDeviceView : public QWidget {
    Q_OBJECT

public:
    static constexpr int FADE_INBETWEEN_DELAY = 100;
    static constexpr int FADE_DURATION = 500;

    explicit ConnectedDeviceView(DotMatrixManager* dot_matrix_manager, QWidget *parent = nullptr);

    void fadeIn();
    void fadeOut();

signals:
    void disconnectButtonClicked();

private:
    DotMatrixManager* dot_matrix_manager;

    QString previous_name;

    QLabel *connected_to_label = nullptr;
    QLabel *name_label = nullptr;
    QPushButton *disconnect_button = nullptr;

    void onDisconnectButtonClicked();
    QString elideNameLabel(const QString& name);
};
