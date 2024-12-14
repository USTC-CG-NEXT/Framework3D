#pragma once
#include "boost/python/numpy/ndarray.hpp"
#include "nvrhi/nvrhi.h"
#include "rich_type_buffer.hpp"
#if USTC_CG_WITH_TORCH
#include "torch/torch.h"
#endif
namespace USTC_CG::node_mass_spring {
class MassSpring;
}

namespace USTC_CG::node_sph_fluid {
class SPHBase;
}

namespace USTC_CG::node_character_animation {
class Animator;
}

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace socket_aliases {
using Float1Buffer = pxr::VtArray<float>;
using pxr::GfVec2f = pxr::VtArray<pxr::GfVec2f>;
using pxr::VtVec3fArray = pxr::VtArray<pxr::GfVec3f>;
using Float4Buffer = pxr::VtArray<pxr::GfVec4f>;
using pxr::VtArray<int> = pxr::VtArray<int>;
using Int2Buffer = pxr::VtArray<pxr::GfVec2i>;
using Int3Buffer = pxr::VtArray<pxr::GfVec3i>;
using Int4Buffer = pxr::VtArray<pxr::GfVec4i>;
using Float2 = pxr::GfVec2f;
using Float3 = pxr::GfVec3f;
using Float4 = pxr::GfVec4f;
using Int2 = pxr::GfVec2i;
using Int3 = pxr::GfVec3i;
using Int4 = pxr::GfVec4i;
using Bool = bool;
using Int = int;
using Float = float;
using String = std::string;
using Geometry = Geometry;
using Lights = LightArray;
using Layer = pxr::UsdStageRefPtr;
using Camera = CameraArray;
using Meshes = MeshArray;
using Texture = nvrhi::TextureHandle;
using Buffer = nvrhi::BufferHandle;
using Materials = MaterialMap;
using PyObj = boost::python::object;
using AccelStruct = nvrhi::rt::AccelStructHandle;
using SocketGroup = SocketGroup;
using NumpyArray = boost::python::numpy::ndarray;
using MassSpringSocket = std::shared_ptr<node_mass_spring::MassSpring>;
using SPHFluidSocket = std::shared_ptr<node_sph_fluid::SPHBase>;
using AnimatorSocket = std::shared_ptr<node_character_animation::Animator>;

#if USTC_CG_WITH_TORCH
using TorchTensor = torch::Tensor;
#endif

}  // namespace socket_aliases
USTC_CG_NAMESPACE_CLOSE_SCOPE