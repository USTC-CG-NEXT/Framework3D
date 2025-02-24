#pragma once

#include "USTC_CG.h"
#include "buffer_socket_types.hpp"
#include "make_standard_type.hpp"
USTC_CG_NAMESPACE_OPEN_SCOPE
namespace decl {

DECLARE_SOCKET_TYPE(Geometry)
DECLARE_SOCKET_TYPE(MassSpringSocket)
DECLARE_SOCKET_TYPE(SPHFluidSocket)
DECLARE_SOCKET_TYPE(AnimatorSocket)

}  // namespace decl

USTC_CG_NAMESPACE_CLOSE_SCOPE