#include "PixelGridModel.hpp"
#include <cstdint>

PixelGridModel::PixelGridModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_grid(ROWS * COLS, Qt::white)
{}

inline std::uint16_t PixelGridModel::idx(const QPoint& point) const { return static_cast<std::uint16_t>(point.y() * COLS + point.x()); }
inline std::uint16_t PixelGridModel::idx(std::uint8_t x, std::uint8_t y) const { return static_cast<std::uint16_t>(y * COLS + x); }

int PixelGridModel::rowCount(const QModelIndex&) const { return ROWS; }
int PixelGridModel::columnCount(const QModelIndex&) const { return COLS; }

QVariant PixelGridModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) { return {}; }

    if (role == ColorRole) {
        return m_grid[idx(static_cast<std::uint8_t>(index.column()), static_cast<std::uint8_t>(index.row()))];
    }

    return {};
}

QColor PixelGridModel::currentColor() const { return m_currentColor; }
void PixelGridModel::setCurrentColor(const QColor& c) { m_currentColor = c; }

const QColor& PixelGridModel::getPixel(const QPoint& point) { return m_grid[idx(point)]; }
const QColor& PixelGridModel::getPixel(std::uint8_t x, std::uint8_t y) { return m_grid[idx(x, y)]; }

void PixelGridModel::setPixel(const QPoint& point, const QColor& color) {
    setPixel(
        static_cast<std::uint8_t>(point.x()),
        static_cast<std::uint8_t>(point.y()),
        color
    );
}

void PixelGridModel::setPixel(std::uint8_t x, std::uint8_t y, const QColor& color) {
    if (x >= COLS || y >= ROWS) { return; }

    m_grid[idx(x, y)] = color;
    const QModelIndex i = index(y, x);

    emit dataChanged(i, i, {ColorRole});
    emit pixelChanged(x, y, color);
}

void PixelGridModel::clear() {
    m_grid.fill(Qt::black);
    emit dataChanged(index(0, 0), index(ROWS - 1, COLS - 1), {ColorRole});
    emit cleared();
}
