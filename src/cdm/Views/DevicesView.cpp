#include <QBoxLayout>
#include <QAbstractItemModel>

#include "DevicesView.hpp"
#include "DeviceItemDelegate.hpp"

DevicesView::DevicesView(DevicesModel* view_model, QWidget *parent) : QWidget(parent), view_model(view_model) {
    deviceList = new QListView(this);
    deviceList->setModel(view_model);
    deviceList->setItemDelegate(new DeviceItemDelegate(this));
    deviceList->setMouseTracking(true);
    deviceList->setStyleSheet(R"(
        QListView {
            color: white;
            background-color: #1A1A1A;
            border: none;
        }
    )");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(deviceList);
    setLayout(layout);

    connect(deviceList, &QListView::doubleClicked, this, [view_model](const QModelIndex& index) {
        if (!index.isValid()) { return; }

        view_model->manager->connectToDevice(view_model->manager->discoveredDevices()[static_cast<std::size_t>(index.row())]);
    });
}
