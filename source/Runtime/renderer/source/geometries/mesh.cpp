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
#include "Scene/SceneTypes.slang"
#include "nvrhi/utils.h"
#include "pxr/base/gf/vec2f.h"
#include "pxr/imaging/hd/extComputationUtils.h"
#include "pxr/imaging/hd/instancer.h"
#include "pxr/imaging/hd/meshUtil.h"
#include "pxr/imaging/hd/smoothNormals.h"

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
    // create model buffer (constant buffer, CPU writable)
    auto device = RHI::get_device();
    nvrhi::BufferDesc buffer_desc =
        nvrhi::BufferDesc{}
            .setByteSize(sizeof(GfMatrix4f))
            .setStructStride(sizeof(GfMatrix4f))
            .setInitialState(nvrhi::ResourceStates::ShaderResource)
            .setCpuAccess(nvrhi::CpuAccessMode::Write)
            .setDebugName("modelBuffer");
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
    HdDirtyBits dirtyBits,
    HdRenderParam* param)
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
                _primvarSourceMap[pv.name] = {
                    GetPrimvar(sceneDelegate, pv.name), interp
                };

                if (pv.name == pxr::TfToken("UVMap") ||
                    pv.name == pxr::TfToken("st")

                ) {
                    texcoordBuffer =
                        static_cast<Hd_USTC_CG_RenderParam*>(param)
                            ->InstanceCollection->vertex_pool.allocate(
                                points.size() * 2);
                    texcoordBuffer->write_data(_primvarSourceMap[pv.name]
                                                   .data.Get<VtVec2fArray>()
                                                   .data());
                }
            }
        }
    }
}

void Hd_USTC_CG_Mesh::create_gpu_resources(Hd_USTC_CG_RenderParam* render_param)
{
    auto device = RHI::get_device();

    auto descriptor_table =
        render_param->InstanceCollection->get_descriptor_table();

    auto& vertex_buffer_pool = render_param->InstanceCollection->vertex_pool;
    vertexBuffer = vertex_buffer_pool.allocate(points.size() * 3);
    vertexBuffer->write_data(points.data());

    indexBuffer = render_param->InstanceCollection->index_pool.allocate(
        triangulatedIndices.size() * 3);
    indexBuffer->write_data(triangulatedIndices.data());

    normalBuffer = render_param->InstanceCollection->vertex_pool.allocate(
        computedNormals.size() * 3);

    normalBuffer->write_data(computedNormals.data());
    {
        std::lock_guard lock(execution_launch_mutex);
        nvrhi::rt::AccelStructDesc blas_desc;
        nvrhi::rt::GeometryDesc geometry_desc;
        geometry_desc.geometryType = nvrhi::rt::GeometryType::Triangles;
        nvrhi::rt::GeometryTriangles triangles;
        triangles.setVertexBuffer(vertexBuffer->get_device_buffer())
            .setVertexOffset(vertexBuffer->offset)
            .setIndexBuffer(indexBuffer->get_device_buffer())
            .setIndexOffset(indexBuffer->offset)
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

        m_CommandList->open();
        nvrhi::utils::BuildBottomLevelAccelStruct(
            m_CommandList, BLAS, blas_desc);
        m_CommandList->close();
        device->executeCommandList(m_CommandList);
        device->waitForIdle();
        device->runGarbageCollection();
    }

    MeshDesc mesh_desc;
    mesh_desc.vbOffset = vertexBuffer->index();
    mesh_desc.ibOffset = indexBuffer->index();
    mesh_desc.normalOffset = normalBuffer->index();
    mesh_desc.vbBufferIndex = 0;

    mesh_desc_buffer = render_param->InstanceCollection->mesh_pool.allocate(1);
    mesh_desc_buffer->write_data(&mesh_desc);
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
        transforms.push_back(GfMatrix4d(1.0f));
    }

    auto& rt_instance_pool = render_param->InstanceCollection->rt_instance_pool;
    std::vector<nvrhi::rt::InstanceDesc> instances;
    instances.resize(transforms.size());

    rt_instanceBuffer = rt_instance_pool.allocate(instances.size());

    instanceBuffer = render_param->InstanceCollection->instance_pool.allocate(
        transforms.size());

    for (int i = 0; i < transforms.size(); ++i) {
        // Combine the local transform and the instance transform.

        GfMatrix4f mat = transform * GfMatrix4f(transforms[i]);
        GfMatrix4f mat_transposed = mat.GetTranspose();

        nvrhi::rt::InstanceDesc instanceDesc;
        instanceDesc.blasDeviceAddress = BLAS->getDeviceAddress();
        instanceDesc.instanceMask = 1;
        instanceDesc.flags =
            nvrhi::rt::InstanceFlags::TriangleFrontCounterclockwise;

        memcpy(
            instanceDesc.transform,
            mat_transposed.data(),
            sizeof(nvrhi::rt::AffineTransform));

        instanceDesc.instanceID = instanceBuffer->index() + i;
        instances[i] = instanceDesc;

        GeometryInstanceData instance_data;
        instance_data.geometryID = mesh_desc_buffer->index();
        instance_data.materialID = 0;
        memcpy(&instance_data.transform, mat.data(), sizeof(pxr::GfMatrix4f));
        instanceBuffer->write_data(&instance_data, i);
    }
    render_param->InstanceCollection->set_require_rebuild_tlas();
    rt_instanceBuffer->write_data(instances.data());

    draw_indirect =
        render_param->InstanceCollection->draw_indirect_pool.allocate(1);
    nvrhi::DrawIndexedIndirectArguments args;
    args.indexCount = triangulatedIndices.size() * 3;
    args.instanceCount = instances.size();
    args.startIndexLocation = indexBuffer->index();
    args.baseVertexLocation = 0;
    args.startInstanceLocation = instanceBuffer->index();

    draw_indirect->write_data(&args);
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

    if (HdChangeTracker::IsVisibilityDirty(*dirtyBits, id)) {
        _sharedData.visible = sceneDelegate->GetVisible(id);
    }

    if (*dirtyBits & HdChangeTracker::DirtyMaterialId) {
        _SetMaterialId(sceneDelegate, this);
    }

    bool requires_rebuild_blas =
        HdChangeTracker::IsPrimvarDirty(*dirtyBits, id, HdTokens->points) ||
        HdChangeTracker::IsTopologyDirty(*dirtyBits, id);

    bool requires_rebuild_tlas =
        requires_rebuild_blas ||
        HdChangeTracker::IsInstancerDirty(*dirtyBits, id) ||
        HdChangeTracker::IsTransformDirty(*dirtyBits, id) ||
        HdChangeTracker::IsVisibilityDirty(*dirtyBits, id);

    if (HdChangeTracker::IsPrimvarDirty(*dirtyBits, id, HdTokens->points)) {
        VtValue value = sceneDelegate->Get(id, HdTokens->points);
        points = value.Get<VtVec3fArray>();

        _normalsValid = false;
    }

    if (!points.empty()) {
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
            // TODO: fill instance matrix buffer
            transform = GfMatrix4f(sceneDelegate->GetTransform(id));
        }

        if (HdChangeTracker::IsPrimvarDirty(
                *dirtyBits, id, HdTokens->normals) ||
            HdChangeTracker::IsPrimvarDirty(*dirtyBits, id, HdTokens->widths) ||
            HdChangeTracker::IsPrimvarDirty(
                *dirtyBits, id, HdTokens->primvar)) {
            _UpdatePrimvarSources(sceneDelegate, *dirtyBits, renderParam);
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

            _normalsValid = true;
        }
        _UpdateComputedPrimvarSources(sceneDelegate, *dirtyBits);
        if (!points.empty()) {
            if (requires_rebuild_blas) {
                create_gpu_resources(
                    static_cast<Hd_USTC_CG_RenderParam*>(renderParam));
            }

            if (requires_rebuild_tlas) {
                if (IsVisible()) {
                    updateTLAS(
                        static_cast<Hd_USTC_CG_RenderParam*>(renderParam),
                        sceneDelegate,
                        dirtyBits);
                }
            }
        }

        *dirtyBits &= ~HdChangeTracker::AllSceneDirtyBits;
    }
}

void Hd_USTC_CG_Mesh::Finalize(HdRenderParam* renderParam)
{
    vertexBuffer = nullptr;
    indexBuffer = nullptr;
    texcoordBuffer = nullptr;
    normalBuffer = nullptr;
    BLAS = nullptr;
    instanceBuffer = nullptr;
    rt_instanceBuffer = nullptr;
    mesh_desc_buffer = nullptr;
    draw_indirect = nullptr;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
