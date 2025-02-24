#include <openvdb/openvdb.h>
#include <openvdb/points/PointDataGrid.h>
#include <openvdb/tools/ParticlesToLevelSet.h>
#include <openvdb/tools/VolumeToMesh.h>

#include "GCore/Components/MeshOperand.h"
#include "GCore/Components/PointsComponent.h"
#include "geom_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(points_to_mesh)
{
    b.add_input<Geometry>("Points");
    b.add_input<float>("Voxel Size").min(0.1).max(5).default_val(0.5f);
    b.add_output<Geometry>("Mesh");
}
using namespace openvdb;

struct WrappingParticleList {
    using PosType = openvdb::Vec3R;
    WrappingParticleList(
        const pxr::VtArray<pxr::GfVec3f>& points_vertices,
        const pxr::VtArray<float>& points_widths)
        : points_vertices(points_vertices),
          points_widths(points_widths)
    {
    }

    // Return the total number of particles in the list.
    // Always required!
    size_t size() const
    {
        return points_vertices.size();
    }

    // Get the world-space position of the nth particle.
    // Required by rasterizeSpheres().
    void getPos(size_t n, Vec3R& xyz) const
    {
        auto& point = points_vertices[n];
        xyz = { point[0], point[1], point[2] };
    }

    // Get the world-space position and radius of the nth particle.
    // Required by rasterizeSpheres().
    void getPosRad(size_t n, Vec3R& xyz, Real& radius) const
    {
        radius = points_widths[n];
        auto& point = points_vertices[n];
        xyz = { point[0], point[1], point[2] };
    }

    // Get the world-space position, radius and velocity of the nth particle.
    // Required by rasterizeTrails().
    void getPosRadVel(size_t n, Vec3R& xyz, Real& radius, Vec3R& velocity)
        const;

    const pxr::VtArray<pxr::GfVec3f>& points_vertices;
    const pxr::VtArray<float>& points_widths;
};

NODE_EXECUTION_FUNCTION(points_to_mesh)
{
    auto points_geometry = params.get_input<Geometry>("Points");

    auto points = points_geometry.get_component<PointsComponent>();

    if (!points || points->get_vertices().empty()) {
        throw std::runtime_error("Input does not contain points");
    }

    auto mesh_geometry = Geometry();
    auto mesh_component = std::make_shared<MeshComponent>(&mesh_geometry);
    mesh_geometry.attach_component(mesh_component);

    pxr::VtArray<pxr::GfVec3f> points_vertices = points->get_vertices();
    pxr::VtArray<float> points_widths = points->get_width();
    if (points_widths.empty()) {
        points_widths.resize(points_vertices.size(), 0.1f);
    }

    pxr::VtArray<pxr::GfVec3f> mesh_vertices;
    pxr::VtArray<int> mesh_faceVertexCounts;
    pxr::VtArray<int> mesh_faceVertexIndices;

    auto pa = WrappingParticleList(points_vertices, points_widths);

    float voxelSize = params.get_input<float>("Voxel Size");
    voxelSize = std::clamp(voxelSize, .001f, std::numeric_limits<float>::max());
    openvdb::FloatGrid::Ptr grid =
        openvdb::createLevelSet<openvdb::FloatGrid>(voxelSize);
    openvdb::tools::ParticlesToLevelSet<openvdb::FloatGrid> raster(*grid);

    raster.setGrainSize(1);  // a value of zero disables threading
    raster.rasterizeSpheres(pa);

    std::vector<openvdb::Vec3s> converted_points;
    std::vector<openvdb::Vec4I> converted_quads;
    std::vector<openvdb::Vec3I> converted_triangles;

    openvdb::tools::volumeToMesh(
        *grid,
        converted_points,
        converted_triangles,
        converted_quads,
        0.f,
        0.0f);

    for (auto&& converted_point : converted_points) {
        mesh_vertices.emplace_back(
            converted_point[0], converted_point[1], converted_point[2]);
    }

    for (int i = 0; i < converted_quads.size(); ++i) {
        mesh_faceVertexCounts.emplace_back(4);
        for (int j = 0; j < 4; ++j) {
            mesh_faceVertexIndices.emplace_back(converted_quads[i][j]);
        }
    }

    for (int i = 0; i < converted_triangles.size(); ++i) {
        mesh_faceVertexCounts.emplace_back(3);
        for (int j = 0; j < 3; ++j) {
            mesh_faceVertexIndices.emplace_back(converted_triangles[i][j]);
        }
    }

    mesh_component->set_vertices(mesh_vertices);
    mesh_component->set_face_vertex_counts(mesh_faceVertexCounts);
    mesh_component->set_face_vertex_indices(mesh_faceVertexIndices);

    params.set_output("Mesh", std::move(mesh_geometry));

    return true;
}

NODE_DECLARATION_UI(points_to_mesh);
NODE_DEF_CLOSE_SCOPE
