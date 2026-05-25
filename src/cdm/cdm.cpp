#include "AppWindow.hpp"
#include "qtresource.h"
#include "theme.hpp"
#include <QApplication>

Q_DECLARE_METATYPE(std::uint8_t)

int main(int argc, char *argv[]) {
    qRegisterMetaType<std::uint8_t>("std::uint8_t");

    QApplication app(argc, argv);

    app.setPalette(Theme::buildPalette());
    app.setStyleSheet(QString(R"(
        QPushButton {
            color: %1;
            background-color: %2;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
        }

        QPushButton:hover { background-color: %3; }
        QPushButton:pressed { background-color: %4; }
        QPushButton:disabled { background-color: %5; }

        QScrollBar:vertical {
            border: none;
            background: transparent;
            width: 8px;
            margin: 0px 0px 0px 0px; 
        }

        QScrollBar::handle:vertical {
            background: #555555; 
            min-height: 30px;
            border-radius: 12px;
        }

        QScrollBar::handle:vertical:hover {
            background: #777777; 
        }

        QScrollBar::add-line:vertical, 
        QScrollBar::sub-line:vertical {
            border: none;
            background: none;
            height: 0px;
        }

        QScrollBar::add-page:vertical, 
        QScrollBar::sub-page:vertical {
            background: none;
        }
    )").arg(Theme::text.name(), Theme::primary.name(), Theme::primary_hovered.name(), Theme::primary_pressed.name(), Theme::primary_disabled.name()));

    AppWindow window;
    window.show();

    return app.exec();
}
