#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QPoint>
#include <cstdint>
#include "../Models/PixelGridModel.hpp"

class PixelCell : public QGraphicsRectItem {
public:
    PixelCell(std::uint8_t rows, std::uint8_t columns, QGraphicsItem* parent = nullptr);

    std::uint8_t getRows() const;
    std::uint8_t getColumns() const;

private:
    std::uint8_t rows;
    std::uint8_t columns;
};

class PixelGridView : public QGraphicsView {
    Q_OBJECT

public:
    explicit PixelGridView(PixelGridModel* model, QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    PixelGridModel* m_model;
    QGraphicsScene* m_scene;
    PixelCell* m_cells[PixelGridModel::ROWS][PixelGridModel::COLS];

    bool m_painting = false;
    bool m_erasing  = false;

    QPoint viewToCell(const QPoint& viewPos) const;
    void paintCell(const QPoint& pixel, const QColor& color);
    void paintCell(std::uint8_t x, std::uint8_t y, const QColor& color);

private slots:
    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>&  roles);
};
