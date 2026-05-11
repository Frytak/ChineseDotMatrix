#include "DevicesView.hpp"
#include "DeviceView.hpp"
#include "qboxlayout.h"
#include "qlistwidget.h"

DevicesView::DevicesView(DevicesViewModel* view_model, QWidget *parent) : QWidget(parent), _view_model(view_model) {
    deviceList = new QListWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(deviceList);
    setLayout(layout);

    connect(_view_model, &DevicesViewModel::deviceAdded, _view_model, [this](DeviceViewModel* item) {
        deviceList->clear();
        for (const auto& device : _view_model->discoveredDevices()) {
            auto list_item = new QListWidgetItem(deviceList);
            DeviceView* device_widget = new DeviceView(device, deviceList);
            deviceList->addItem(list_item);
            list_item->setSizeHint(device_widget->sizeHint());
            deviceList->setItemWidget(list_item, device_widget);
        }
    });

    connect(deviceList, &QListWidget::itemDoubleClicked, _view_model, [this](QListWidgetItem* item) {
        auto item_widget = static_cast<DeviceView*>(deviceList->itemWidget(item));
        _view_model->connectToDevice(item_widget->_view_model);
    });
}
