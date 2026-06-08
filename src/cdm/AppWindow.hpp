#pragma once

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include "DotMatrixManager.hpp"
#include "LayeredSvgDisplay.hpp"
#include "Models/DevicesModel.hpp"
#include "Models/PixelGridModel.hpp"
#include "Views/DevicesView.hpp"
#include "Views/PixelGridView.hpp"

class AppWindow : public QWidget {
    Q_OBJECT

public:
    explicit AppWindow(QWidget *parent = nullptr);
    static constexpr auto backgrond_color = QColor(0x1A, 0x1A, 0x1A);
    static constexpr auto text_color = QColor(0xFF, 0xFF, 0xFF);

private slots:
    void onScanButtonClicked();
    void onRotate0ButtonClicked();
    void onRotate1ButtonClicked();
    void onPickColorButtonClicked();
    void onClearButtonClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    // D-Bus connection
    std::shared_ptr<sdbus::IConnection> conn;
    DotMatrixManager* dot_matrix_manager;

    DevicesModel* devices_model;
    PixelGridModel* pixel_grid_model;

    DevicesView* devices_view;
    PixelGridView* pixel_grid_view;

    QPushButton* scan_button;
    QPushButton* rotate0_button;
    QPushButton* rotate1_button;
    QPushButton* color_button;
    QPushButton* clear_button;

    LayeredSvgDisplay* devicesBackground;
    LayeredSvgDisplay* connectedDeviceBackground;
    LayeredSvgDisplay* toolbarBackground;

    void updateColorButton(const QColor& c);
};
