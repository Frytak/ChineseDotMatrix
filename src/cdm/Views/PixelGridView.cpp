#include "PixelGridView.hpp"
#include <QMouseEvent>
#include <cstdlib>

PixelCell::PixelCell(std::uint8_t rows, std::uint8_t columns, QGraphicsItem* parent)
    : QGraphicsRectItem(0, 0, PixelGridModel::PIXEL_SIZE, PixelGridModel::PIXEL_SIZE, parent)
    , rows(rows)
    , columns(columns)
{
    setBrush(Qt::black);
    setPen(QPen(QColor(50, 50, 50), 0.5));
}

std::uint8_t PixelCell::getRows() const { return rows; }
std::uint8_t PixelCell::getColumns() const { return columns; }



PixelGridView::PixelGridView(PixelGridModel* model, QWidget* parent)
    : QGraphicsView(parent)
    , m_model(model)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing, false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);

    const int W = PixelGridModel::COLS * PixelGridModel::PIXEL_SIZE;
    const int H = PixelGridModel::ROWS * PixelGridModel::PIXEL_SIZE;
    m_scene->setSceneRect(0, 0, W, H);
    setFixedSize(W + 2, H + 2);

    for (std::uint8_t y = 0; y < PixelGridModel::ROWS; ++y) {
        for (std::uint8_t x = 0; x < PixelGridModel::COLS; ++x) {
            auto* cell = new PixelCell(y, x);
            cell->setPos(x * PixelGridModel::PIXEL_SIZE, y * PixelGridModel::PIXEL_SIZE);
            m_scene->addItem(cell);
            m_cells[y][x] = cell;
        }
    }

    connect(m_model, &PixelGridModel::dataChanged, this, &PixelGridView::onDataChanged);
}

void PixelGridView::onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>&  roles) {
    if (!roles.contains(PixelGridModel::ColorRole) && !roles.isEmpty()) return;

    for (int r = topLeft.row(); r <= bottomRight.row(); ++r) {
        for (int c = topLeft.column(); c <= bottomRight.column(); ++c) {
            m_cells[r][c]->setBrush(
                m_model->data(m_model->index(r, c), PixelGridModel::ColorRole).value<QColor>()
            );
        }
    }
}

QPoint PixelGridView::viewToCell(const QPoint& vp) const {
    QPointF sp = mapToScene(vp);
    return {
        static_cast<int>(sp.x()) / PixelGridModel::PIXEL_SIZE,
        static_cast<int>(sp.y()) / PixelGridModel::PIXEL_SIZE
    };
}

void PixelGridView::paintCell(const QPoint& point, const QColor& color) {
    paintCell(
        static_cast<std::uint8_t>(point.x()),
        static_cast<std::uint8_t>(point.y()),
        color
    );
}

void PixelGridView::paintCell(std::uint8_t x, std::uint8_t y, const QColor& color) {
    m_model->setPixel(x, y, color);
}

void PixelGridView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_painting = true; m_erasing = false;
    } else if (event->button() == Qt::RightButton) {
        m_painting = false; m_erasing = true;
    } else return;

    QPoint cell = viewToCell(event->pos());
    QColor color = m_erasing ? Qt::black : m_model->currentColor();
    if (m_model->getPixel(cell) == color) return;

    paintCell(cell, color);
}

void PixelGridView::mouseMoveEvent(QMouseEvent* event) {
    if (!m_painting && !m_erasing) return;

    QPoint cell = viewToCell(event->pos());
    QColor color = m_erasing ? Qt::black : m_model->currentColor();
    if (m_model->getPixel(cell) == color) return;

    paintCell(cell, color);
}

void PixelGridView::mouseReleaseEvent(QMouseEvent* event) {
    const bool leftDone  = (event->button() == Qt::LeftButton  && m_painting);
    const bool rightDone = (event->button() == Qt::RightButton && m_erasing);
    if (!leftDone && !rightDone) return;

    m_painting = false;
    m_erasing  = false;
}
