// #define __GNUC__

#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdSkel/animQuery.h>
#include <pxr/usd/usdSkel/cache.h>
#include <pxr/usd/usdSkel/skeleton.h>

#include <memory>

#include "GCore/Components/MaterialComponent.h"
#include "GCore/Components/MeshOperand.h"
#include "GCore/Components/SkelComponent.h"
#include "GCore/Components/XformComponent.h"
#include "geom_node_base.h"
#include "pxr/usd/usdSkel/animation.h"
#include "pxr/usd/usdSkel/bindingAPI.h"
#include "pxr/usd/usdSkel/skeletonQuery.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(read_usd)
{
    b.add_input<std::string>("File Name").default_val("Default");
    b.add_input<std::string>("Prim Path").default_val("geometry");
    b.add_input<float>("Time Code").default_val(0).min(0).max(240);
    b.add_output<Geometry>("Geometry");
}

NODE_EXECUTION_FUNCTION(read_usd)
{
    auto file_name = params.get_input<std::string>("File Name");
    auto prim_path = params.get_input<std::string>("Prim Path");

    Geometry geometry;
    std::shared_ptr<MeshComponent> mesh =
        std::make_shared<MeshComponent>(&geometry);
    geometry.attach_component(mesh);

    auto t = params.get_input<float>("Time Code");
    pxr::UsdTimeCode time = pxr::UsdTimeCode(t);
    if (t == 0) {
        time = pxr::UsdTimeCode::Default();
    }

    auto stage = pxr::UsdStage::Open(file_name.c_str());

    if (stage) {
        // Here 'c_str' call is necessary since prim_path
        auto sdf_path = pxr::SdfPath(prim_path.c_str());
        pxr::UsdGeomMesh usdgeom = pxr::UsdGeomMesh::Get(stage, sdf_path);
        
        if (usdgeom) {
            mesh->set_mesh_geom(usdgeom);

            pxr::GfMatrix4d final_transform =
                usdgeom.ComputeLocalToWorldTransform(time);

            if (final_transform != pxr::GfMatrix4d().SetIdentity()) {
                auto xform_component =
                    std::make_shared<XformComponent>(&geometry);
                geometry.attach_component(xform_component);

                auto rotation = final_transform.ExtractRotation();
                auto translation = final_transform.ExtractTranslation();
                // TODO: rotation not read.

                xform_component->translation.push_back(
                    pxr::GfVec3f(translation));
                xform_component->rotation.push_back(pxr::GfVec3f(0.0f));
                xform_component->scale.push_back(pxr::GfVec3f(1.0f));
            }
            using namespace pxr;
            UsdSkelBindingAPI binding = UsdSkelBindingAPI(usdgeom);
            SdfPathVector targets;
            binding.GetSkeletonRel().GetTargets(&targets);
            if (targets.size() == 1) {
                auto prim = stage->GetPrimAtPath(targets[0]);

                pxr::UsdSkelSkeleton skeleton(prim);
                if (skeleton) {
                    using namespace pxr;
                    UsdSkelCache skelCache;
                    UsdSkelSkeletonQuery skelQuery =
                        skelCache.GetSkelQuery(skeleton);

                    auto skel_component =
                        std::make_shared<SkelComponent>(&geometry);
                    geometry.attach_component(skel_component);

                    VtArray<GfMatrix4f> xforms;
                    skelQuery.ComputeJointLocalTransforms(&xforms, time);

                    skel_component->localTransforms = xforms;
                    skel_component->jointOrder = skelQuery.GetJointOrder();
                    skel_component->topology = skelQuery.GetTopology();

                    VtArray<float> jointWeight;
                    binding.GetJointWeightsAttr().Get(&jointWeight, time);

                    VtArray<GfMatrix4d> bindTransforms;
                    skeleton.GetBindTransformsAttr().Get(&bindTransforms, time);
                    skel_component->bindTransforms = bindTransforms;

                    VtArray<int> jointIndices;
                    binding.GetJointIndicesAttr().Get(&jointIndices, time);
                    skel_component->jointWeight = jointWeight;
                    skel_component->jointIndices = jointIndices;
                }
                else {
                    log::warning("Unable to read the skeleton.");
                    return false;
                }
            }
        }

        else {
            log::warning("Unable to read the prim.");
            return false;
        }

        // TODO: add material reading
    }
    else {
        // TODO: throw something
    }
    params.set_output("Geometry", std::move(geometry));
    return true;
}

NODE_DECLARATION_UI(read_usd);
NODE_DEF_CLOSE_SCOPE
