#pragma once
#include <string>

#include "diff_optics/api.h"
#include "diff_optics/lens_system.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
OpticalProperty get_optical_property(const std::string& name);

USTC_CG_NAMESPACE_CLOSE_SCOPE