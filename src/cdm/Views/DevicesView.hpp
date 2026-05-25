#pragma once

#include <QWidget>
#include <QListView>

#include "../Models/DevicesModel.hpp"

class DevicesView : public QWidget {
    Q_OBJECT

public:
    explicit DevicesView(DevicesModel* view_model, QWidget *parent = nullptr);

private:
    DevicesModel* view_model;
    QListView* deviceList;
};
