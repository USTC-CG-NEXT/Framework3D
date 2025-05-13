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
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<pxr::GfVec3f> vertices;
        if (mesh.GetPointsAttr())
            mesh.GetPointsAttr().Get(&vertices);
        return vertices;
#else
        return vertices;
#endif
    }

    [[nodiscard]] pxr::VtArray<int> get_face_vertex_counts() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<int> faceVertexCounts;
        if (mesh.GetFaceVertexCountsAttr())
            mesh.GetFaceVertexCountsAttr().Get(&faceVertexCounts);
        return faceVertexCounts;
#else
        return faceVertexCounts;
#endif
    }

    [[nodiscard]] pxr::VtArray<int> get_face_vertex_indices() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<int> faceVertexIndices;
        if (mesh.GetFaceVertexIndicesAttr())
            mesh.GetFaceVertexIndicesAttr().Get(&faceVertexIndices);
        return faceVertexIndices;
#else
        return faceVertexIndices;
#endif
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_normals() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<pxr::GfVec3f> normals;
        if (mesh.GetNormalsAttr())
            mesh.GetNormalsAttr().Get(&normals);
        return normals;
#else
        return normals;
#endif
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_display_color() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<pxr::GfVec3f> displayColor;
        if (mesh.GetDisplayColorAttr())
            mesh.GetDisplayColorAttr().Get(&displayColor);
        return displayColor;
#else
        return displayColor;
#endif
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec2f> get_texcoords_array() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<pxr::GfVec2f> texcoordsArray;
        auto PrimVarAPI = pxr::UsdGeomPrimvarsAPI(mesh);
        auto primvar = PrimVarAPI.GetPrimvar(pxr::TfToken("UVMap"));
        if (primvar)
            primvar.Get(&texcoordsArray);
        return texcoordsArray;
#else
        return texcoordsArray;
#endif
    }

    [[nodiscard]] pxr::VtArray<float> get_control_points() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<float> control_points;
        auto PrimVarAPI = pxr::UsdGeomPrimvarsAPI(mesh);
        auto primvar = PrimVarAPI.GetPrimvar(pxr::TfToken("ControlPoints"));
        if (primvar)
            primvar.Get(&control_points);
        return control_points;
#else
        return controlPoints;
#endif
    }

    [[nodiscard]] pxr::VtArray<float> get_vertex_scalar_quantity(
        const std::string& name) const
    {
        auto it = vertex_scalar_quantities.find(name);
        if (it != vertex_scalar_quantities.end()) {
            return it->second;
        }
        return pxr::VtArray<float>();
    }

    [[nodiscard]] std::vector<std::string> get_vertex_scalar_quantity_names()
        const
    {
        std::vector<std::string> names;
        for (const auto& pair : vertex_scalar_quantities) {
            names.push_back(pair.first);
        }
        return names;
    }

    [[nodiscard]] pxr::VtArray<float> get_face_scalar_quantity(
        const std::string& name) const
    {
        auto it = face_scalar_quantities.find(name);
        if (it != face_scalar_quantities.end()) {
            return it->second;
        }
        return pxr::VtArray<float>();
    }

    [[nodiscard]] std::vector<std::string> get_face_scalar_quantity_names()
        const
    {
        std::vector<std::string> names;
        for (const auto& pair : face_scalar_quantities) {
            names.push_back(pair.first);
        }
        return names;
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_vertex_color_quantity(
        const std::string& name) const
    {
        auto it = vertex_color_quantities.find(name);
        if (it != vertex_color_quantities.end()) {
            return it->second;
        }
        return pxr::VtArray<pxr::GfVec3f>();
    }

    [[nodiscard]] std::vector<std::string> get_vertex_color_quantity_names()
        const
    {
        std::vector<std::string> names;
        for (const auto& pair : vertex_color_quantities) {
            names.push_back(pair.first);
        }
        return names;
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_face_color_quantity(
        const std::string& name) const
    {
        auto it = face_color_quantities.find(name);
        if (it != face_color_quantities.end()) {
            return it->second;
        }
        return pxr::VtArray<pxr::GfVec3f>();
    }

    [[nodiscard]] std::vector<std::string> get_face_color_quantity_names() const
    {
        std::vector<std::string> names;
        for (const auto& pair : face_color_quantities) {
            names.push_back(pair.first);
        }
        return names;
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_vertex_vector_quantity(
        const std::string& name) const
    {
        auto it = vertex_vector_quantities.find(name);
        if (it != vertex_vector_quantities.end()) {
            return it->second;
        }
        return pxr::VtArray<pxr::GfVec3f>();
    }

    [[nodiscard]] std::vector<std::string> get_vertex_vector_quantity_names()
        const
    {
        std::vector<std::string> names;
        for (const auto& pair : vertex_vector_quantities) {
            names.push_back(pair.first);
        }
        return names;
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_face_vector_quantity(
        const std::string& name) const
    {
        auto it = face_vector_quantities.find(name);
        if (it != face_vector_quantities.end()) {
            return it->second;
        }
        return pxr::VtArray<pxr::GfVec3f>();
    }

    [[nodiscard]] std::vector<std::string> get_face_vector_quantity_names()
        const
    {
        std::vector<std::string> names;
        for (const auto& pair : face_vector_quantities) {
            names.push_back(pair.first);
        }
        return names;
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec2f>
    get_face_corner_parameterization_quantity(const std::string& name) const
    {
        auto it = face_corner_parameterization_quantities.find(name);
        if (it != face_corner_parameterization_quantities.end()) {
            return it->second;
        }
        return pxr::VtArray<pxr::GfVec2f>();
    }

    [[nodiscard]] std::vector<std::string>
    get_face_corner_parameterization_quantity_names() const
    {
        std::vector<std::string> names;
        for (const auto& pair : face_corner_parameterization_quantities) {
            names.push_back(pair.first);
        }
        return names;
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec2f>
    get_vertex_parameterization_quantity(const std::string& name) const
    {
        auto it = vertex_parameterization_quantities.find(name);
        if (it != vertex_parameterization_quantities.end()) {
            return it->second;
        }
        return pxr::VtArray<pxr::GfVec2f>();
    }

    [[nodiscard]] std::vector<std::string>
    get_vertex_parameterization_quantity_names() const
    {
        std::vector<std::string> names;
        for (const auto& pair : vertex_parameterization_quantities) {
            names.push_back(pair.first);
        }
        return names;
    }

    void set_vertices(const pxr::VtArray<pxr::GfVec3f>& vertices)
    {
#if USE_USD_SCRATCH_BUFFER
        mesh.CreatePointsAttr().Set(vertices);
#else
        this->vertices = vertices;
#endif
    }

    void set_face_vertex_counts(const pxr::VtArray<int>& face_vertex_counts)
    {
#if USE_USD_SCRATCH_BUFFER
        mesh.CreateFaceVertexCountsAttr().Set(face_vertex_counts);
#else
        this->faceVertexCounts = face_vertex_counts;
#endif
    }

    void set_face_vertex_indices(const pxr::VtArray<int>& face_vertex_indices)
    {
#if USE_USD_SCRATCH_BUFFER
        mesh.CreateFaceVertexIndicesAttr().Set(face_vertex_indices);
#else
        this->faceVertexIndices = face_vertex_indices;
#endif
    }

    void set_normals(const pxr::VtArray<pxr::GfVec3f>& normals)
    {
#if USE_USD_SCRATCH_BUFFER
        mesh.CreateNormalsAttr().Set(normals);
#else
        this->normals = normals;
#endif
    }

    void set_texcoords_array(const pxr::VtArray<pxr::GfVec2f>& texcoords_array)
    {
#if USE_USD_SCRATCH_BUFFER
        auto PrimVarAPI = pxr::UsdGeomPrimvarsAPI(mesh);
        auto primvar = PrimVarAPI.CreatePrimvar(
            pxr::TfToken("UVMap"), pxr::SdfValueTypeNames->TexCoord2fArray);
        primvar.Set(texcoords_array);

        if (get_texcoords_array().size() == get_vertices().size()) {
            primvar.SetInterpolation(pxr::UsdGeomTokens->vertex);
        }
        else {
            primvar.SetInterpolation(pxr::UsdGeomTokens->faceVarying);
        }
#else
        this->texcoordsArray = texcoords_array;
#endif
    }

    void set_control_points(const pxr::VtArray<float>& control_points)
    {
#if USE_USD_SCRATCH_BUFFER
        // TODO: not implemented
        auto PrimVarAPI = pxr::UsdGeomPrimvarsAPI(mesh);
        auto primvar = PrimVarAPI.CreatePrimvar(
            pxr::TfToken("ControlPoints"), pxr::SdfValueTypeNames->FloatArray);
        primvar.Set(control_points);
#else
        this->controlPoints = control_points;
#endif
    }

    void set_display_color(const pxr::VtArray<pxr::GfVec3f>& display_color)
    {
#if USE_USD_SCRATCH_BUFFER
        auto PrimVarAPI = pxr::UsdGeomPrimvarsAPI(mesh);
        pxr::UsdGeomPrimvar colorPrimvar = PrimVarAPI.CreatePrimvar(
            pxr::TfToken("displayColor"), pxr::SdfValueTypeNames->Color3fArray);
        colorPrimvar.SetInterpolation(pxr::UsdGeomTokens->vertex);
        colorPrimvar.Set(display_color);
#else
        this->displayColor = display_color;
#endif
    }

    void set_vertex_scalar_quantities(
        const std::map<std::string, pxr::VtArray<float>>& scalar)
    {
        vertex_scalar_quantities = scalar;
    }

    void set_face_scalar_quantities(
        const std::map<std::string, pxr::VtArray<float>>& scalar)
    {
        face_scalar_quantities = scalar;
    }

    void set_vertex_color_quantities(
        const std::map<std::string, pxr::VtArray<pxr::GfVec3f>>& color)
    {
        vertex_color_quantities = color;
    }

    void set_face_color_quantities(
        const std::map<std::string, pxr::VtArray<pxr::GfVec3f>>& color)
    {
        face_color_quantities = color;
    }

    void set_vertex_vector_quantities(
        const std::map<std::string, pxr::VtArray<pxr::GfVec3f>>& vector)
    {
        vertex_vector_quantities = vector;
    }

    void set_face_vector_quantities(
        const std::map<std::string, pxr::VtArray<pxr::GfVec3f>>& vector)
    {
        face_vector_quantities = vector;
    }

    void set_face_corner_parameterization_quantities(
        const std::map<std::string, pxr::VtArray<pxr::GfVec2f>>&
            parameterization)
    {
        face_corner_parameterization_quantities = parameterization;
    }

    void set_vertex_parameterization_quantities(
        const std::map<std::string, pxr::VtArray<pxr::GfVec2f>>&
            parameterization)
    {
        vertex_parameterization_quantities = parameterization;
    }

    void add_vertex_scalar_quantity(
        const std::string& name,
        const pxr::VtArray<float>& scalar)
    {
        vertex_scalar_quantities[name] = scalar;
    }

    void add_face_scalar_quantity(
        const std::string& name,
        const pxr::VtArray<float>& scalar)
    {
        face_scalar_quantities[name] = scalar;
    }

    void add_vertex_color_quantity(
        const std::string& name,
        const pxr::VtArray<pxr::GfVec3f>& color)
    {
        vertex_color_quantities[name] = color;
    }

    void add_face_color_quantity(
        const std::string& name,
        const pxr::VtArray<pxr::GfVec3f>& color)
    {
        face_color_quantities[name] = color;
    }

    void add_vertex_vector_quantity(
        const std::string& name,
        const pxr::VtArray<pxr::GfVec3f>& vector)
    {
        vertex_vector_quantities[name] = vector;
    }

    void add_face_vector_quantity(
        const std::string& name,
        const pxr::VtArray<pxr::GfVec3f>& vector)
    {
        face_vector_quantities[name] = vector;
    }

    void add_face_corner_parameterization_quantity(
        const std::string& name,
        const pxr::VtArray<pxr::GfVec2f>& parameterization)
    {
        face_corner_parameterization_quantities[name] = parameterization;
    }

    void add_vertex_parameterization_quantity(
        const std::string& name,
        const pxr::VtArray<pxr::GfVec2f>& parameterization)
    {
        vertex_parameterization_quantities[name] = parameterization;
    }

#if USE_USD_SCRATCH_BUFFER
    void set_mesh_geom(const pxr::UsdGeomMesh& usdgeom);
    pxr::UsdGeomMesh get_usd_mesh() const;
#endif
    void append_mesh(const std::shared_ptr<MeshComponent>& mesh);

   private:
#if USE_USD_SCRATCH_BUFFER
    pxr::UsdGeomMesh mesh;

#else
    // Local cache for mesh attributes when USD cache is not enabled
    pxr::VtArray<pxr::GfVec3f> vertices;
    pxr::VtArray<int> faceVertexCounts;
    pxr::VtArray<int> faceVertexIndices;
    pxr::VtArray<pxr::GfVec3f> normals;
    pxr::VtArray<pxr::GfVec3f> displayColor;
    pxr::VtArray<pxr::GfVec2f> texcoordsArray;
    pxr::VtArray<float> controlPoints;
#endif

    // After adding these quantities, you need to modify the copy() function

    // Quantities for polyscope
    // Edge quantities are not supported because the indexing is not clear
    std::map<std::string, pxr::VtArray<float>> vertex_scalar_quantities;
    std::map<std::string, pxr::VtArray<float>> face_scalar_quantities;
    // pxr::VtArray<pxr::VtArray<float>> edge_scalar_quantities;
    // pxr::VtArray<pxr::VtArray<float>> halfedge_scalar_quantities
    std::map<std::string, pxr::VtArray<pxr::GfVec3f>> vertex_color_quantities;
    std::map<std::string, pxr::VtArray<pxr::GfVec3f>> face_color_quantities;
    std::map<std::string, pxr::VtArray<pxr::GfVec3f>> vertex_vector_quantities;
    std::map<std::string, pxr::VtArray<pxr::GfVec3f>> face_vector_quantities;
    std::map<std::string, pxr::VtArray<pxr::GfVec2f>>
        face_corner_parameterization_quantities;
    std::map<std::string, pxr::VtArray<pxr::GfVec2f>>
        vertex_parameterization_quantities;
    // pxr::VtArray<pxr::VtArray<pxr::GfVec3f>> misc_quantities_nodes;
    // pxr::VtArray<pxr::VtArray<pxr::GfVec2i>> misc_quantities_edges;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
