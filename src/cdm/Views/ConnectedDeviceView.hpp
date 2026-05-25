#pragma once

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "../DotMatrixManager.hpp"
#include "qpushbutton.h"

class ConnectedDeviceView : public QWidget {
    Q_OBJECT

public:
    explicit ConnectedDeviceView(DotMatrixManager* dot_matrix_manager, QWidget *parent = nullptr);

signals:
    void disconnectButtonClicked();

private:
    DotMatrixManager* dot_matrix_manager;

    QLabel *name_label = nullptr;
    QPushButton *disconnect_button = nullptr;

    void onDisconnectButtonClicked();
    QString elideNameLabel(const QString& name);
};
