#include "GCore/Components/MeshOperand.h"

#include "GCore/GOP.h"
#include "global_stage.hpp"
#include "stage/stage.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
MeshComponent::MeshComponent(Geometry* attached_operand)
    : GeometryComponent(attached_operand)
{
#if USE_USD_SCRATCH_BUFFER

    scratch_buffer_path = pxr::SdfPath(
        "/scratch_buffer/mesh_component_" +
        std::to_string(reinterpret_cast<long long>(this)));
    mesh =
        pxr::UsdGeomMesh::Define(g_stage->get_usd_stage(), scratch_buffer_path);
    pxr::UsdGeomImageable(mesh).MakeInvisible();
#endif
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
#if USE_USD_SCRATCH_BUFFER

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
#endif
GeometryComponentHandle MeshComponent::copy(Geometry* operand) const
{
    auto ret = std::make_shared<MeshComponent>(operand);
#if USE_USD_SCRATCH_BUFFER
    copy_prim(mesh.GetPrim(), ret->mesh.GetPrim());
    pxr::UsdGeomImageable(mesh).MakeInvisible();
#else
    ret->set_vertices(this->vertices);
    ret->set_face_vertex_counts(this->faceVertexCounts);
    ret->set_face_vertex_indices(this->faceVertexIndices);
    ret->set_normals(this->normals);
    ret->set_display_color(this->displayColor);
#endif
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

#if USE_USD_SCRATCH_BUFFER
void MeshComponent::set_mesh_geom(const pxr::UsdGeomMesh& usdgeom)
{
    copy_prim(usdgeom.GetPrim(), mesh.GetPrim());
    pxr::UsdGeomImageable(mesh).MakeInvisible();
}

pxr::UsdGeomMesh MeshComponent::get_usd_mesh() const
{
    return mesh;
}
#endif

void MeshComponent::append_mesh(const std::shared_ptr<MeshComponent>& mesh)

{
    auto this_vertices = get_vertices();
    auto this_face_vertex_indices = get_face_vertex_indices();

    auto that_vertices = mesh->get_vertices();

    auto that_face_vertex_indices = mesh->get_face_vertex_indices();

    int this_index_offset = this_vertices.size();

    this_vertices.resize(this_vertices.size() + that_vertices.size());
    memcpy(
        this_vertices.data() + this_index_offset,
        that_vertices.data(),
        that_vertices.size() * sizeof(pxr::GfVec3f));

    // Append face vertex indices
    for (auto& index : that_face_vertex_indices) {
        this_face_vertex_indices.push_back(index + this_index_offset);
    }

    set_vertices(this_vertices);
    set_face_vertex_indices(this_face_vertex_indices);

    auto this_vertex_counts = get_face_vertex_counts();
    auto this_vertex_counts_size = this_vertex_counts.size();
    auto that_vertex_counts = mesh->get_face_vertex_counts();

    this_vertex_counts.resize(
        this_vertex_counts.size() + that_vertex_counts.size());

    memcpy(
        this_vertex_counts.data() + this_vertex_counts_size,
        that_vertex_counts.data(),
        that_vertex_counts.size() * sizeof(int));

    set_face_vertex_counts(this_vertex_counts);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
