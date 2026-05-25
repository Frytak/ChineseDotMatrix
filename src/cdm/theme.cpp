#include "theme.hpp"

namespace Theme {
    const QColor background = QColor(0x1A, 0x1A, 0x1F);
    const QColor surface = background.darker(120);
    const QColor primary = QColor(0x2B, 0x66, 0xC4);
    const QColor primary_pressed = primary.darker(120);
    const QColor primary_hovered = primary.lighter(120);
    const QColor primary_disabled = QColor(0x41, 0x49, 0x57);
    const QColor text = QColor(0xFF, 0xFF, 0xFF);

    QPalette buildPalette() {
        QPalette palette;

        // Backgrounds
        palette.setColor(QPalette::Window, background);
        palette.setColor(QPalette::Base, surface);
        palette.setColor(QPalette::Button, primary);
        palette.setColor(QPalette::Highlight, primary_hovered);
        palette.setColor(QPalette::ToolTipBase, surface);

        // Text
        palette.setColor(QPalette::Text, text);
        palette.setColor(QPalette::WindowText, text);
        palette.setColor(QPalette::ButtonText, text);
        palette.setColor(QPalette::HighlightedText, text);
        palette.setColor(QPalette::ToolTipText, text);

        return palette;
    }
}
