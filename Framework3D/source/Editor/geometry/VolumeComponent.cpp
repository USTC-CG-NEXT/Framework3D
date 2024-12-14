#include "GCore/Components/VolumeComponent.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
GeometryComponentHandle VolumeComponet::copy(Geometry* operand) const
{
    (void)(operand);
    return {};
}

std::string VolumeComponet::to_string() const
{
    return std::string("VolumeComponet");
}
USTC_CG_NAMESPACE_CLOSE_SCOPE
