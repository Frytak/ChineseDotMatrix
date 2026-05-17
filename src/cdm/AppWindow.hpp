#pragma once

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include "DotMatrixManager.hpp"
#include "Models/DevicesModel.hpp"
#include "Models/PixelGridModel.hpp"
#include "Views/DevicesView.hpp"
#include "Views/PixelGridView.hpp"

class AppWindow : public QWidget {
    Q_OBJECT

public:
    explicit AppWindow(QWidget *parent = nullptr);

private slots:
    void onScanButtonClicked();
    void onRotate0ButtonClicked();
    void onRotate1ButtonClicked();
    void onPickColorButtonClicked();
    void onClearButtonClicked();
    void testButtonClicked();

private:
    // D-Bus connection
    std::shared_ptr<sdbus::IConnection> conn;
    DotMatrixManager* dot_matrix_manager;

    DevicesModel* devicesModel;
    PixelGridModel* pixelGridModel;

    DevicesView* devicesView;
    PixelGridView* pixelGridView;

    QPushButton* scanButton;
    QPushButton* rotate0Button;
    QPushButton* rotate1Button;
    QPushButton* testButton;
    QPushButton* colorButton;
    QPushButton* clearButton;

    void updateColorButton(const QColor& c);
};
