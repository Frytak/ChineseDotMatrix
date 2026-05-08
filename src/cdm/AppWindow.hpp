#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include "../lib/lib.hpp"

class AppWindow : public QWidget {
    Q_OBJECT

public:
    explicit AppWindow(QWidget *parent = nullptr);

private slots:
    void onScanButtonClicked();
    void onDeviceFound(const QString& deviceName);
    void onRotate0ButtonClicked();
    void onRotate1ButtonClicked();

private:
    // D-Bus connection
    std::shared_ptr<sdbus::IConnection> conn;

    // DotMatrix device (if connected)
    std::optional<DotMatrix> matrix;

    QPushButton *scanButton;
    QListWidget *deviceList;
    QPushButton *rotate0Button;
    QPushButton *rotate1Button;
};

#endif
