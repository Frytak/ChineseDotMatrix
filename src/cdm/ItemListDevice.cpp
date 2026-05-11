#include "ItemListDevice.hpp"
#include "QVBoxLayout"
#include "QLabel"

ItemListDevice::ItemListDevice(const bluez::BluezDevice& device, QWidget *parent) : QWidget(parent), object_path(device.getObjectPath()), name(QString::fromStdString(device.getAlias())) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel(name, this);
    label->setStyleSheet("color: white;");
    layout->addWidget(label);
    setLayout(layout);
}

const sdbus::ObjectPath& ItemListDevice::getObjectPath() const {
    return object_path;
}

const QString& ItemListDevice::getName() const {
    return name;
}
