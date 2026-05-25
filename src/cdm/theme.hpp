#pragma once

#include <QColor>
#include <QPalette>

namespace Theme {
    extern const QColor background;
    extern const QColor surface;
    extern const QColor primary;
    extern const QColor primary_pressed;
    extern const QColor primary_hovered;
    extern const QColor primary_disabled;
    extern const QColor text;

    QPalette buildPalette();
}
