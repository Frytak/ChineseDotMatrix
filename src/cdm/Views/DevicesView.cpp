#include <QBoxLayout>
#include <QAbstractItemModel>

#include "../theme.hpp"
#include "DevicesView.hpp"
#include "DeviceItemDelegate.hpp"

DevicesView::DevicesView(DevicesModel* view_model, QWidget *parent) : QWidget(parent), view_model(view_model) {
    deviceList = new QListView;
    deviceList->setModel(view_model);
    deviceList->setItemDelegate(new DeviceItemDelegate(this));
    deviceList->setMouseTracking(true);
    deviceList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    deviceList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    deviceList->setStyleSheet(QString(R"(
        QListView {
            color: %1;
            background-color: transparent;
            border: none;
        }
    )").arg(Theme::text.name()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(deviceList);
    setLayout(layout);

    connect(deviceList, &QListView::doubleClicked, this, [view_model](const QModelIndex& index) {
        if (!index.isValid()) { return; }

        view_model->manager->connectToDevice(view_model->manager->discoveredDevices()[static_cast<std::size_t>(index.row())]);
    });
}
