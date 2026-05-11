#include "qlistwidget.h"
#include "qtmetamacros.h"
#include "qwidget.h"
#include "../ViewModels/DevicesViewModel.hpp"

class DevicesView : public QWidget {
    Q_OBJECT

public:
    explicit DevicesView(DevicesViewModel* view_model, QWidget *parent = nullptr);

private:
    DevicesViewModel* _view_model;

    QListWidget *deviceList;
};
