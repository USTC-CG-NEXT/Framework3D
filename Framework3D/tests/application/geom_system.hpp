#pragma once

#include "GCore/geom_payload.hpp"
#include "nodes/system/node_system_dl.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

class GeomSystem : public NodeDynamicLoadingSystem {
   public:
    GeomSystem();
    void execute() const override;
};

inline GeomSystem::GeomSystem()
{
    register_cpp_types<GeomPayload>();
}

inline void GeomSystem::execute() const
{
    NodeDynamicLoadingSystem::execute();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE