//
// Copyright 2020 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
// #define __GNUC__
#include "mesh.h"

#include <iostream>

#include "../api.h"
#include "../instancer.h"
#include "../renderParam.h"
#include "Logger/Logger.h"
#include "nvrhi/utils.h"
#include "pxr/base/gf/vec2f.h"
#include "pxr/imaging/hd/extComputationUtils.h"
#include "pxr/imaging/hd/instancer.h"
#include "pxr/imaging/hd/meshUtil.h"
#include "pxr/imaging/hd/smoothNormals.h"

#include "Scene/SceneTypes.slang"

USTC_CG_NAMESPACE_OPEN_SCOPE
class Hd_USTC_CG_RenderParam;
using namespace pxr;
Hd_USTC_CG_Mesh::Hd_USTC_CG_Mesh(const SdfPath& id)
    : HdMesh(id),
      _cullStyle(HdCullStyleDontCare),
      _doubleSided(false),
      _normalsValid(false),
      _adjacencyValid(false),
      _refined(false)
{
}

Hd_USTC_CG_Mesh::~Hd_USTC_CG_Mesh()
{
}

HdDirtyBits Hd_USTC_CG_Mesh::GetInitialDirtyBitsMask() const
{
    int mask =
        HdChangeTracker::Clean | HdChangeTracker::InitRepr |
        HdChangeTracker::DirtyPoints | HdChangeTracker::DirtyTopology |
        HdChangeTracker::DirtyTransform | HdChangeTracker::DirtyVisibility |
        HdChangeTracker::DirtyCullStyle | HdChangeTracker::DirtyDoubleSided |
        HdChangeTracker::DirtyDisplayStyle | HdChangeTracker::DirtySubdivTags |
        HdChangeTracker::DirtyPrimvar | HdChangeTracker::DirtyNormals |
        HdChangeTracker::DirtyInstancer | HdChangeTracker::DirtyMaterialId;

    return (HdDirtyBits)mask;
}

HdDirtyBits Hd_USTC_CG_Mesh::_PropagateDirtyBits(HdDirtyBits bits) const
{
    return bits;
}

TfTokenVector Hd_USTC_CG_Mesh::_UpdateComputedPrimvarSources(
    HdSceneDelegate* sceneDelegate,
    HdDirtyBits dirtyBits)
{
    HD_TRACE_FUNCTION();

    const SdfPath& id = GetId();

    // Get all the dirty computed primvars
    HdExtComputationPrimvarDescriptorVector dirtyCompPrimvars;
    for (size_t i = 0; i < HdInterpolationCount; ++i) {
        HdExtComputationPrimvarDescriptorVector compPrimvars;
        auto interp = static_cast<HdInterpolation>(i);
        compPrimvars =
            sceneDelegate->GetExtComputationPrimvarDescriptors(GetId(), interp);

        for (const auto& pv : compPrimvars) {
            if (HdChangeTracker::IsPrimvarDirty(dirtyBits, id, pv.name)) {
                dirtyCompPrimvars.emplace_back(pv);
            }
        }
    }

    if (dirtyCompPrimvars.empty()) {
        return TfTokenVector();
    }

    HdExtComputationUtils::ValueStore valueStore =
        HdExtComputationUtils::GetComputedPrimvarValues(
            dirtyCompPrimvars, sceneDelegate);

    TfTokenVector compPrimvarNames;
    // Update local primvar map and track the ones that were computed
    for (const auto& compPrimvar : dirtyCompPrimvars) {
        const auto it = valueStore.find(compPrimvar.name);
        if (!TF_VERIFY(it != valueStore.end())) {
            continue;
        }

        compPrimvarNames.emplace_back(compPrimvar.name);
        _primvarSourceMap[compPrimvar.name] = { it->second,
                                                compPrimvar.interpolation };
    }

    return compPrimvarNames;
}

void Hd_USTC_CG_Mesh::_UpdatePrimvarSources(
    HdSceneDelegate* sceneDelegate,
    HdDirtyBits dirtyBits)
{
    HD_TRACE_FUNCTION();
    const SdfPath& id = GetId();

    HdPrimvarDescriptorVector primvars;
    for (size_t i = 0; i < HdInterpolationCount; ++i) {
        auto interp = static_cast<HdInterpolation>(i);
        primvars = GetPrimvarDescriptors(sceneDelegate, interp);
        for (const HdPrimvarDescriptor& pv : primvars) {
            if (HdChangeTracker::IsPrimvarDirty(dirtyBits, id, pv.name) &&
                pv.name != HdTokens->points) {
                log::info(("Primvar source " + pv.name.GetString()).c_str());
                _primvarSourceMap[pv.name] = {
                    GetPrimvar(sceneDelegate, pv.name), interp
                };
            }
        }
    }
}

void Hd_USTC_CG_Mesh::updateBLAS(Hd_USTC_CG_RenderParam* render_param)
{
    auto device = RHI::get_device();

    nvrhi::BufferDesc buffer_desc;

    buffer_desc.byteSize = points.size() * 3 * sizeof(float);
    buffer_desc.format = nvrhi::Format::RGB32_FLOAT;
    buffer_desc.isAccelStructBuildInput = true;
    buffer_desc.cpuAccess = nvrhi::CpuAccessMode::Write;
    vertexBuffer = device->createBuffer(buffer_desc);

    buffer_desc.byteSize = triangulatedIndices.size() * 3 * sizeof(unsigned);
    buffer_desc.format = nvrhi::Format ::R32_UINT;
    indexBuffer = device->createBuffer(buffer_desc);

    auto buffer = device->mapBuffer(vertexBuffer, nvrhi::CpuAccessMode::Write);
    memcpy(buffer, points.data(), points.size() * 3 * sizeof(float));
    device->unmapBuffer(vertexBuffer);

    buffer = device->mapBuffer(indexBuffer, nvrhi::CpuAccessMode::Write);
    memcpy(
        buffer,
        triangulatedIndices.data(),
        triangulatedIndices.size() * 3 * sizeof(unsigned));
    device->unmapBuffer(indexBuffer);

    nvrhi::rt::AccelStructDesc blas_desc;
    nvrhi::rt::GeometryDesc geometry_desc;
    geometry_desc.geometryType = nvrhi::rt::GeometryType::Triangles;
    nvrhi::rt::GeometryTriangles triangles;
    triangles.setVertexBuffer(vertexBuffer)
        .setIndexBuffer(indexBuffer)
        .setIndexCount(triangulatedIndices.size() * 3)
        .setVertexCount(points.size())
        .setVertexStride(3 * sizeof(float))
        .setVertexFormat(nvrhi::Format::RGB32_FLOAT)
        .setIndexFormat(nvrhi::Format::R32_UINT);
    geometry_desc.setTriangles(triangles);
    blas_desc.addBottomLevelGeometry(geometry_desc);
    blas_desc.isTopLevel = false;
    BLAS = device->createAccelStruct(blas_desc);

    auto m_CommandList = device->createCommandList();
    {
        std::lock_guard lock(render_param->TLAS->edit_instances_mutex);
        m_CommandList->open();
        nvrhi::utils::BuildBottomLevelAccelStruct(
            m_CommandList, BLAS, blas_desc);
        m_CommandList->close();
    }
    device->executeCommandList(m_CommandList);
}

void Hd_USTC_CG_Mesh::updateTLAS(
    Hd_USTC_CG_RenderParam* render_param,
    HdSceneDelegate* sceneDelegate,
    HdDirtyBits* dirtyBits)
{
    _UpdateInstancer(sceneDelegate, dirtyBits);
    const SdfPath& id = GetId();

    HdInstancer::_SyncInstancerAndParents(
        sceneDelegate->GetRenderIndex(), GetInstancerId());

    VtMatrix4dArray transforms;
    if (!GetInstancerId().IsEmpty()) {
        // Retrieve instance transforms from the instancer.
        HdRenderIndex& renderIndex = sceneDelegate->GetRenderIndex();
        HdInstancer* instancer = renderIndex.GetInstancer(GetInstancerId());
        transforms = static_cast<Hd_USTC_CG_Instancer*>(instancer)
                         ->ComputeInstanceTransforms(GetId());
    }
    else {
        // If there's no instancer, add a single instance with transform
        // I.
        transforms.push_back(GfMatrix4d(1.0));
    }

    auto& instances = render_param->TLAS->acquire_instances_to_edit(this);
    instances.clear();
    instances.resize(transforms.size());

    for (int i = 0; i < transforms.size(); ++i) {
        // Combine the local transform and the instance transform.
        GfMatrix4f matf =
            (transform * GfMatrix4f(transforms[i])).GetTranspose();

        nvrhi::rt::InstanceDesc instanceDesc;
        instanceDesc.bottomLevelAS = BLAS;
        instanceDesc.instanceMask = 1;
        instanceDesc.flags =
            nvrhi::rt::InstanceFlags::TriangleFrontCounterclockwise;

        memcpy(
            instanceDesc.transform,
            matf.data(),
            sizeof(nvrhi::rt::AffineTransform));
        instances[i] = instanceDesc;
    }
}

void Hd_USTC_CG_Mesh::_InitRepr(
    const TfToken& reprToken,
    HdDirtyBits* dirtyBits)
{
}

void Hd_USTC_CG_Mesh::_SetMaterialId(
    HdSceneDelegate* delegate,
    Hd_USTC_CG_Mesh* rprim)
{
    SdfPath const& newMaterialId = delegate->GetMaterialId(rprim->GetId());
    if (rprim->GetMaterialId() != newMaterialId) {
        rprim->SetMaterialId(newMaterialId);
    }
}

void Hd_USTC_CG_Mesh::Sync(
    HdSceneDelegate* sceneDelegate,
    HdRenderParam* renderParam,
    HdDirtyBits* dirtyBits,
    const TfToken& reprToken)
{
    _dirtyBits = *dirtyBits;
    HD_TRACE_FUNCTION();
    HF_MALLOC_TAG_FUNCTION();

    _MeshReprConfig::DescArray descs = _GetReprDesc(reprToken);

    const SdfPath& id = GetId();
    std::string path = id.GetText();

    if (*dirtyBits & HdChangeTracker::DirtyMaterialId) {
        _SetMaterialId(sceneDelegate, this);
    }

    bool requires_rebuild_blas =
        HdChangeTracker::IsPrimvarDirty(*dirtyBits, id, HdTokens->points) ||
        HdChangeTracker::IsTopologyDirty(*dirtyBits, id);

    bool requires_rebuild_tlas =
        requires_rebuild_blas ||
        HdChangeTracker::IsInstancerDirty(*dirtyBits, id) ||
        HdChangeTracker::IsTransformDirty(*dirtyBits, id);

    if (HdChangeTracker::IsPrimvarDirty(*dirtyBits, id, HdTokens->points)) {
        VtValue value = sceneDelegate->Get(id, HdTokens->points);
        points = value.Get<VtVec3fArray>();

        _normalsValid = false;
    }

    if (HdChangeTracker::IsTopologyDirty(*dirtyBits, id)) {
        topology = GetMeshTopology(sceneDelegate);
        HdMeshUtil meshUtil(&topology, GetId());
        meshUtil.ComputeTriangleIndices(
            &triangulatedIndices, &trianglePrimitiveParams);
        _normalsValid = false;
        _adjacencyValid = false;
    }
    if (HdChangeTracker::IsInstancerDirty(*dirtyBits, id) ||
        HdChangeTracker::IsTransformDirty(*dirtyBits, id)) {
        transform = GfMatrix4f(sceneDelegate->GetTransform(id));
    }

    if (HdChangeTracker::IsPrimvarDirty(*dirtyBits, id, HdTokens->normals) ||
        HdChangeTracker::IsPrimvarDirty(*dirtyBits, id, HdTokens->widths) ||
        HdChangeTracker::IsPrimvarDirty(*dirtyBits, id, HdTokens->primvar)) {
        _UpdatePrimvarSources(sceneDelegate, *dirtyBits);
        _texcoordsClean = false;
    }

    if (!_adjacencyValid) {
        _adjacency.BuildAdjacencyTable(&topology);
        _adjacencyValid = true;
        // If we rebuilt the adjacency table, force a rebuild of normals.
        _normalsValid = false;
    }

    if (!_normalsValid) {
        computedNormals = Hd_SmoothNormals::ComputeSmoothNormals(
            &_adjacency, points.size(), points.cdata());

        assert(points.size() == computedNormals.size());
    }
    _UpdateComputedPrimvarSources(sceneDelegate, *dirtyBits);
    if (!points.empty()) {
        if (requires_rebuild_blas) {
            updateBLAS(static_cast<Hd_USTC_CG_RenderParam*>(renderParam));
        }

        if (requires_rebuild_tlas) {
            updateTLAS(
                static_cast<Hd_USTC_CG_RenderParam*>(renderParam),
                sceneDelegate,
                dirtyBits);
        }
    }
    *dirtyBits &= ~HdChangeTracker::AllSceneDirtyBits;
}

void Hd_USTC_CG_Mesh::Finalize(HdRenderParam* renderParam)
{
    static_cast<Hd_USTC_CG_RenderParam*>(renderParam)
        ->TLAS->removeInstance(this);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
