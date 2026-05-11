#include "qtmetamacros.h"
#include "qwidget.h"
#include "../ViewModels/DeviceViewModel.hpp"

class DeviceView : public QWidget {
    Q_OBJECT

public:
    explicit DeviceView(DeviceViewModel* view_model, QWidget *parent = nullptr);

private:
    DeviceViewModel* _view_model;
    
    friend class DevicesView;
};
