#include "DeviceItemDelegate.hpp"
#include "../Models/DevicesModel.hpp"
#include <QPainter>

DeviceItemDelegate::DeviceItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {};

void DeviceItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();

    bool isConnected = index.data(DevicesModel::IsConnectedRole).toBool();
    bool isHovered   = option.state & QStyle::State_MouseOver;
    bool isSelected  = option.state & QStyle::State_Selected;

    if (isConnected) {
        painter->fillRect(option.rect, isHovered ? QColor("#145a28") : QColor("#1a6b3a"));
    } else if (isSelected) {
        painter->fillRect(option.rect, QColor("#0057ae"));
    } else if (isHovered) {
        painter->fillRect(option.rect, QColor("#2d2d2d"));
    }

    painter->setPen(Qt::white);
    painter->drawText(option.rect.adjusted(12, 0, 0, 0), Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
    painter->restore();
}

QSize DeviceItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const {
    return QSize(0, 40);
}
