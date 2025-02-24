#pragma once
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/vt/array.h>
#include <pxr/usd/usdGeom/mesh.h>

#include <string>

#include "GCore/Components.h"
#include "GCore/GOP.h"
#include "pxr/usd/usdGeom/primvarsAPI.h"
#include "pxr/usd/usdGeom/xform.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct GEOMETRY_API MeshComponent : public GeometryComponent {
    explicit MeshComponent(Geometry* attached_operand);

    ~MeshComponent() override;

    void apply_transform(const pxr::GfMatrix4d& transform) override
    {
        auto vertices = get_vertices();
        for (auto& vertex : vertices) {
            vertex = pxr::GfVec3f(transform.Transform(vertex));
        }
        set_vertices(vertices);
    }

    std::string to_string() const override;

    GeometryComponentHandle copy(Geometry* operand) const override;
    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_vertices() const
    {
        pxr::VtArray<pxr::GfVec3f> vertices;
        if (mesh.GetPointsAttr())
            mesh.GetPointsAttr().Get(&vertices);
        return vertices;
    }

    [[nodiscard]] pxr::VtArray<int> get_face_vertex_counts() const
    {
        pxr::VtArray<int> faceVertexCounts;
        if (mesh.GetFaceVertexCountsAttr())
            mesh.GetFaceVertexCountsAttr().Get(&faceVertexCounts);
        return faceVertexCounts;
    }

    [[nodiscard]] pxr::VtArray<int> get_face_vertex_indices() const
    {
        pxr::VtArray<int> faceVertexIndices;
        if (mesh.GetFaceVertexIndicesAttr())
            mesh.GetFaceVertexIndicesAttr().Get(&faceVertexIndices);
        return faceVertexIndices;
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_normals() const
    {
        pxr::VtArray<pxr::GfVec3f> normals;
        if (mesh.GetNormalsAttr())
            mesh.GetNormalsAttr().Get(&normals);
        return normals;
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_display_color() const
    {
        pxr::VtArray<pxr::GfVec3f> displayColor;
        if (mesh.GetDisplayColorAttr())
            mesh.GetDisplayColorAttr().Get(&displayColor);
        return displayColor;
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec2f> get_texcoords_array() const
    {
        pxr::VtArray<pxr::GfVec2f> texcoordsArray;
        auto PrimVarAPI = pxr::UsdGeomPrimvarsAPI(mesh);
        auto primvar = PrimVarAPI.GetPrimvar(pxr::TfToken("UVMap"));
        if (primvar)
            primvar.Get(&texcoordsArray);
        return texcoordsArray;
    }

    [[nodiscard]] pxr::VtArray<pxr::VtArray<float>>
    get_vertex_scalar_quantities() const
    {
        return vertex_scalar_quantities;
    }

    [[nodiscard]] pxr::VtArray<pxr::VtArray<float>> get_face_scalar_quantities()
        const
    {
        return face_scalar_quantities;
    }

    [[nodiscard]] pxr::VtArray<pxr::VtArray<pxr::GfVec3f>>
    get_vertex_color_quantities() const
    {
        return vertex_color_quantities;
    }

    [[nodiscard]] pxr::VtArray<pxr::VtArray<pxr::GfVec3f>>
    get_face_color_quantities() const
    {
        return face_color_quantities;
    }

    [[nodiscard]] pxr::VtArray<pxr::VtArray<pxr::GfVec3f>>
    get_vertex_vector_quantities() const
    {
        return vertex_vector_quantities;
    }

    [[nodiscard]] pxr::VtArray<pxr::VtArray<pxr::GfVec3f>>
    get_face_vector_quantities() const
    {
        return face_vector_quantities;
    }

    [[nodiscard]] pxr::VtArray<pxr::VtArray<pxr::GfVec2f>>
    get_face_corner_parameterization_quantities() const
    {
        return face_corner_parameterization_quantities;
    }

    [[nodiscard]] pxr::VtArray<pxr::VtArray<pxr::GfVec2f>>
    get_vertex_parameterization_quantities() const
    {
        return vertex_parameterization_quantities;
    }

    void set_vertices(const pxr::VtArray<pxr::GfVec3f>& vertices)
    {
        mesh.CreatePointsAttr().Set(vertices);
    }

    void set_face_vertex_counts(const pxr::VtArray<int>& face_vertex_counts)
    {
        mesh.CreateFaceVertexCountsAttr().Set(face_vertex_counts);
    }

    void set_face_vertex_indices(const pxr::VtArray<int>& face_vertex_indices)
    {
        mesh.CreateFaceVertexIndicesAttr().Set(face_vertex_indices);
    }

    void set_normals(const pxr::VtArray<pxr::GfVec3f>& normals)
    {
        mesh.CreateNormalsAttr().Set(normals);
    }
    void set_texcoords_array(const pxr::VtArray<pxr::GfVec2f>& texcoords_array)
    {
        auto PrimVarAPI = pxr::UsdGeomPrimvarsAPI(mesh);
        auto primvar = PrimVarAPI.CreatePrimvar(
            pxr::TfToken("UVMap"), pxr::SdfValueTypeNames->TexCoord2fArray);
        primvar.Set(texcoords_array);

        // Here only consider two modes
        if (get_texcoords_array().size() == get_vertices().size()) {
            primvar.SetInterpolation(pxr::UsdGeomTokens->vertex);
        }
        else {
            primvar.SetInterpolation(pxr::UsdGeomTokens->faceVarying);
        }
    }

    void set_display_color(const pxr::VtArray<pxr::GfVec3f>& display_color)
    {
        auto PrimVarAPI = pxr::UsdGeomPrimvarsAPI(mesh);
        pxr::UsdGeomPrimvar colorPrimvar = PrimVarAPI.CreatePrimvar(
            pxr::TfToken("displayColor"), pxr::SdfValueTypeNames->Color3fArray);
        colorPrimvar.SetInterpolation(pxr::UsdGeomTokens->vertex);
        colorPrimvar.Set(display_color);
    }

    void set_vertex_scalar_quantities(
        const pxr::VtArray<pxr::VtArray<float>>& scalar)
    {
        vertex_scalar_quantities = scalar;
    }

    void set_face_scalar_quantities(
        const pxr::VtArray<pxr::VtArray<float>>& scalar)
    {
        face_scalar_quantities = scalar;
    }

    void set_vertex_color_quantities(
        const pxr::VtArray<pxr::VtArray<pxr::GfVec3f>>& color)
    {
        vertex_color_quantities = color;
    }

    void set_face_color_quantities(
        const pxr::VtArray<pxr::VtArray<pxr::GfVec3f>>& color)
    {
        face_color_quantities = color;
    }

    void set_vertex_vector_quantities(
        const pxr::VtArray<pxr::VtArray<pxr::GfVec3f>>& vector)
    {
        vertex_vector_quantities = vector;
    }

    void set_face_vector_quantities(
        const pxr::VtArray<pxr::VtArray<pxr::GfVec3f>>& vector)
    {
        face_vector_quantities = vector;
    }

    void set_face_corner_parameterization_quantities(
        const pxr::VtArray<pxr::VtArray<pxr::GfVec2f>>& parameterization)
    {
        face_corner_parameterization_quantities = parameterization;
    }

    void set_vertex_parameterization_quantities(
        const pxr::VtArray<pxr::VtArray<pxr::GfVec2f>>& parameterization)
    {
        vertex_parameterization_quantities = parameterization;
    }

    void add_vertex_scalar_quantity(const pxr::VtArray<float>& scalar)
    {
        vertex_scalar_quantities.push_back(scalar);
    }

    void add_face_scalar_quantity(const pxr::VtArray<float>& scalar)
    {
        face_scalar_quantities.push_back(scalar);
    }

    void add_vertex_color_quantity(const pxr::VtArray<pxr::GfVec3f>& color)
    {
        vertex_color_quantities.push_back(color);
    }

    void add_face_color_quantity(const pxr::VtArray<pxr::GfVec3f>& color)
    {
        face_color_quantities.push_back(color);
    }

    void add_vertex_vector_quantity(const pxr::VtArray<pxr::GfVec3f>& vector)
    {
        vertex_vector_quantities.push_back(vector);
    }

    void add_face_vector_quantity(const pxr::VtArray<pxr::GfVec3f>& vector)
    {
        face_vector_quantities.push_back(vector);
    }

    void add_face_corner_parameterization_quantity(
        const pxr::VtArray<pxr::GfVec2f>& parameterization)
    {
        face_corner_parameterization_quantities.push_back(parameterization);
    }

    void add_vertex_parameterization_quantity(
        const pxr::VtArray<pxr::GfVec2f>& parameterization)
    {
        vertex_parameterization_quantities.push_back(parameterization);
    }

    void set_mesh_geom(const pxr::UsdGeomMesh& usdgeom);
    pxr::UsdGeomMesh get_usd_mesh() const;
    void append_mesh(const std::shared_ptr<MeshComponent>& mesh);

   private:
    pxr::UsdGeomMesh mesh;

    // After adding these quantities, you need to modify the copy() function

    // Quantities for polyscope
    // Edge quantities are not supported because the indexing is not clear
    pxr::VtArray<pxr::VtArray<float>> vertex_scalar_quantities;
    pxr::VtArray<pxr::VtArray<float>> face_scalar_quantities;
    // pxr::VtArray<pxr::VtArray<float>> edge_scalar_quantities;
    // pxr::VtArray<pxr::VtArray<float>> halfedge_scalar_quantities
    pxr::VtArray<pxr::VtArray<pxr::GfVec3f>> vertex_color_quantities;
    pxr::VtArray<pxr::VtArray<pxr::GfVec3f>> face_color_quantities;
    pxr::VtArray<pxr::VtArray<pxr::GfVec3f>> vertex_vector_quantities;
    pxr::VtArray<pxr::VtArray<pxr::GfVec3f>> face_vector_quantities;
    pxr::VtArray<pxr::VtArray<pxr::GfVec2f>>
        face_corner_parameterization_quantities;
    pxr::VtArray<pxr::VtArray<pxr::GfVec2f>> vertex_parameterization_quantities;
    // pxr::VtArray<pxr::VtArray<pxr::GfVec3f>> misc_quantities_nodes;
    // pxr::VtArray<pxr::VtArray<pxr::GfVec2i>> misc_quantities_edges;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
