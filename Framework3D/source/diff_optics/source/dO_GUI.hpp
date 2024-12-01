#pragma once

#include <GUI/widget.h>
#include <GUI/window.h>
#include <diff_optics/api.h>

#include <memory>

namespace USTC_CG {
class LensSystemGUI;
class LensSystem;
}  // namespace USTC_CG

USTC_CG_NAMESPACE_OPEN_SCOPE
class DiffOpticsGUI : public IWidget {
   public:
    explicit DiffOpticsGUI(LensSystem* lens_system);
    bool BuildUI() override;

   protected:
    bool Begin() override
    {
        return true;
    }

    void End() override
    {
    }

    LensSystem* lens_system;
    std::unique_ptr<LensSystemGUI> lens_gui;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE