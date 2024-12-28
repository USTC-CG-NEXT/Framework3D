#include "GCore/Components/MeshOperand.h"

#include "GCore/GOP.h"
#include "global_stage.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
MeshComponent::MeshComponent(Geometry* attached_operand)
    : GeometryComponent(attached_operand)
{
    scratch_buffer_path = pxr::SdfPath(
        "/scratch_buffer/mesh_component_" +
        std::to_string(reinterpret_cast<long long>(this)));
    mesh =
        pxr::UsdGeomMesh::Define(g_stage->get_usd_stage(), scratch_buffer_path);
    pxr::UsdGeomImageable(mesh).MakeInvisible();
}

MeshComponent::~MeshComponent()
{
}

std::string MeshComponent::to_string() const
{
    std::ostringstream out;
    // Loop over the faces and vertices and print the data
    out << "Topology component. "
        << "Vertices count " << get_vertices().size()
        << ". Face vertices count " << get_face_vertex_counts().size()
        << ". Face vertex indices " << get_face_vertex_indices().size() << ".";
    return out.str();
}

using namespace pxr;
void CopyPrimvar(const UsdGeomPrimvar& sourcePrimvar, const UsdPrim& destPrim)
{
    // Create or get the corresponding primvar on the destination prim
    UsdGeomPrimvar destPrimvar = UsdGeomPrimvarsAPI(destPrim).CreatePrimvar(
        sourcePrimvar.GetName(),
        sourcePrimvar.GetTypeName(),
        sourcePrimvar.GetInterpolation());

    // Copy the primvar value
    VtValue value;
    if (sourcePrimvar.Get(&value)) {
        destPrimvar.Set(value);
    }

    // Copy the element size if it exists
    int elementSize = sourcePrimvar.GetElementSize();
    if (elementSize > 0) {
        destPrimvar.SetElementSize(elementSize);
    }
}

void copy_prim(const pxr::UsdPrim& from, const pxr::UsdPrim& to)
{
    for (pxr::UsdAttribute attr : from.GetPrim().GetAuthoredAttributes()) {
        // Copy attribute value
        pxr::VtValue value;

        pxr::UsdGeomPrimvar sourcePrimvar(attr);
        if (sourcePrimvar) {
            // It's a primvar, copy it as a primvar
            CopyPrimvar(sourcePrimvar, to);
        }
        else {
            if (attr.Get(&value)) {
                to.GetPrim()
                    .CreateAttribute(attr.GetName(), attr.GetTypeName())
                    .Set(value);
            }
        }
    }
}

GeometryComponentHandle MeshComponent::copy(Geometry* operand) const
{
    auto ret = std::make_shared<MeshComponent>(operand);
    copy_prim(this->mesh.GetPrim(), ret->mesh.GetPrim());
    ret->set_vertex_scalar_quantities(this->vertex_scalar_quantities);
    ret->set_face_scalar_quantities(this->face_scalar_quantities);
    ret->set_vertex_color_quantities(this->vertex_color_quantities);
    ret->set_face_color_quantities(this->face_color_quantities);
    ret->set_vertex_vector_quantities(this->vertex_vector_quantities);
    ret->set_face_vector_quantities(this->face_vector_quantities);
    ret->set_face_corner_parameterization_quantities(
        this->face_corner_parameterization_quantities);
    ret->set_vertex_parameterization_quantities(
        this->vertex_parameterization_quantities);
    return ret;
}

void MeshComponent::set_mesh_geom(const pxr::UsdGeomMesh& usdgeom)
{
    copy_prim(usdgeom.GetPrim(), mesh.GetPrim());

    pxr::UsdGeomImageable(mesh).MakeInvisible();
}

pxr::UsdGeomMesh MeshComponent::get_usd_mesh() const
{
    return mesh;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
