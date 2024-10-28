#include "GUI/widget.h"

#include "RHI/DeviceManager/DeviceManager.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
void IWidget::set_window(Window* window)
{
    this->window = window;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE