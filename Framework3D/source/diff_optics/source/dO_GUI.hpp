#pragma once

#include <GUI/widget.h>
#include <diff_optics/api.h>

namespace USTC_CG {
class LensSystem;
}

USTC_CG_NAMESPACE_OPEN_SCOPE
class DiffOpticsGUI : public IWidget {
   public:
    DiffOpticsGUI(LensSystem* lens_system) : lens_system(lens_system)
    {
    }
    bool BuildUI() override;

    LensSystem* lens_system;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE