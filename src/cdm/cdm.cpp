#include "AppWindow.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setStyleSheet(
        "AppWindow {"
        "  background-color: #1A1A1A;"
        "}"
        "QListWidget {"
        "  color: white;"
        "  background-color: #1A1A1A;"
        "}"
        "QPushButton {"
        "  background-color: #2196F3;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 16px;"
        "}"
        "QPushButton:hover { background-color: #1976D2; }"
        "QPushButton:pressed { background-color: #0D47A1; }"
        "QPushButton:disabled { background-color: #aaa; }"
    );

    AppWindow window;
    window.show();

    return app.exec();
}
