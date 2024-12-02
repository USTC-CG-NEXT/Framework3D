#pragma once

#include "GUI/widget.h"
#include "api.h"
#include "lens_system.hpp"
#include "lens_system_compiler.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
std::unique_ptr<IWidget> createDiffOpticsGUI(LensSystem*);

USTC_CG_NAMESPACE_CLOSE_SCOPE
