#pragma once

#include <string>

#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class Window;

class IWidget {
   public:
    IWidget()
    {
    }
    virtual ~IWidget() = default;
    virtual bool BuildUI() = 0;

    void set_window(Window* window);

   protected:
    // Give a widget ability to create another widget.
    Window* window;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
