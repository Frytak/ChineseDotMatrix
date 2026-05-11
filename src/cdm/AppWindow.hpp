#pragma once

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include "../lib/DotMatrix.hpp"
#include "ViewModels/DevicesViewModel.hpp"
#include "Views/DevicesView.hpp"

class AppWindow : public QWidget {
    Q_OBJECT

public:
    explicit AppWindow(QWidget *parent = nullptr);

private slots:
    void onScanButtonClicked();
    void onRotate0ButtonClicked();
    void onRotate1ButtonClicked();
    void testButtonClicked();

private:
    // D-Bus connection
    std::shared_ptr<sdbus::IConnection> conn;

    DevicesViewModel* devicesViewModel;
    QPushButton* scanButton;
    QPushButton* rotate0Button;
    QPushButton* rotate1Button;
    QPushButton* testButton;
    DevicesView* devicesView;
};
