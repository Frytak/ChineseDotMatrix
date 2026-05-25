#pragma once
#include "qpoint.h"
#include <QAbstractTableModel>
#include <QColor>
#include <QVector>
#include <cstdint>

class PixelGridModel : public QAbstractTableModel {
    Q_OBJECT

public:
    static constexpr std::uint8_t ROWS = 32;
    static constexpr std::uint8_t COLS = 32;
    static constexpr int PIXEL_SIZE = 20;

    enum PixelRoles {
        ColorRole = Qt::UserRole + 1
    };

    explicit PixelGridModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QColor primaryColor() const;
    void setPrimaryColor(const QColor& c);

    QColor secondaryColor() const;
    void setSecondaryColor(const QColor& c);

    const QColor& getPixel(const QPoint& point);
    const QColor& getPixel(std::uint8_t x, std::uint8_t y);
    void setPixel(const QPoint& point, const QColor& color);
    void setPixel(std::uint8_t x, std::uint8_t y, const QColor& color);

    void clear();

signals:
    void pixelChanged(std::uint8_t x, std::uint8_t y, const QColor& color);
    void cleared();

private:
    QVector<QColor> grid;
    QColor primary_color = Qt::black;
    QColor secondary_color = Qt::black;

    std::uint16_t idx(const QPoint& point) const;
    std::uint16_t idx(std::uint8_t x, std::uint8_t y) const;
};
