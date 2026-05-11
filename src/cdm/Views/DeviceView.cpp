#include "DeviceView.hpp"
#include "qboxlayout.h"
#include "qlabel.h"

DeviceView::DeviceView(DeviceViewModel* view_model, QWidget *parent) : QWidget(parent), _view_model(view_model) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel(_view_model->name(), this);
    label->setStyleSheet("color: white;");
    layout->addWidget(label);
    setLayout(layout);
}
