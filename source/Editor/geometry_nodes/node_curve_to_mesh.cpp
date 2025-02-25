#include "GCore/Components/CurveComponent.h"
#include "GCore/Components/MeshOperand.h"
#include "geom_node_base.h"
#include "pxr/base/gf/matrix3f.h"
#include "pxr/base/gf/rotation.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(curve_to_mesh)
{
    b.add_input<Geometry>("Curve");
    b.add_input<Geometry>("Profile Curve");

    b.add_output<Geometry>("Mesh");
}

NODE_EXECUTION_FUNCTION(curve_to_mesh)
{
    Geometry mesh_geom = Geometry::CreateMesh();

    auto curve_input = params.get_input<Geometry>("Curve");
    auto profile_curve_input = params.get_input<Geometry>("Profile Curve");

    auto curve = curve_input.get_component<CurveComponent>();
    auto profile_curve = profile_curve_input.get_component<CurveComponent>();

    // Suppose both curve are linear
    using namespace pxr;

    // Calculate the transformation

    // The curve must have a normal.
    pxr::VtArray<GfVec3f> curve_normals = curve->get_curve_normals();
    // Only rotation is needed here.

    auto guide_curve_verts = curve->get_vertices();
    auto profile_curve_verts = profile_curve->get_vertices();

    bool guide_curve_periodic = curve->get_periodic();

    auto vert_count = guide_curve_verts.size();

    auto mesh = mesh_geom.get_component<MeshComponent>();

    VtArray<pxr::GfVec2f> texcoords_array;
    VtArray<int> face_vertex_counts;
    VtArray<pxr::GfVec3f> verticies;
    VtArray<int> face_vertex_indices;

    for (int i = 0; i < guide_curve_verts.size(); ++i) {
        GfVec3f normal = curve_normals[i].GetNormalized();
        GfVec3f tangent;

        int this_vert_id = i;
        int next_vert_id = (i + 1) % vert_count;
        int prev_vert_id = (i - 1) % vert_count;

        bool first_of_periodic = guide_curve_periodic && i == 0;

        bool last_of_periodic =
            guide_curve_periodic && i == guide_curve_verts.size() - 1;

        bool first_of_nonperiodic = !guide_curve_periodic && i == 0;

        bool last_of_nonperiodic =
            !guide_curve_periodic && i == guide_curve_verts.size() - 1;

        if (first_of_nonperiodic) {
            tangent = guide_curve_verts[1] - guide_curve_verts[0];
        }
        else if (last_of_nonperiodic) {
            tangent = guide_curve_verts[guide_curve_verts.size() - 1] -
                      guide_curve_verts[guide_curve_verts.size() - 2];
        }
        else {
            auto vec1 = guide_curve_verts[next_vert_id] -
                        guide_curve_verts[this_vert_id];
            auto vec2 = guide_curve_verts[this_vert_id] -
                        guide_curve_verts[prev_vert_id];

            auto l1 = vec1.Normalize();
            auto l2 = vec2.Normalize();
            auto weight_1 = l2 / (l1 + l2);

            tangent = weight_1 * vec2 + (1 - weight_1) * vec1;
        }
        tangent.Normalize();
        auto bitangent = GfCross(normal, tangent);

        GfMatrix3f tbn;

        tbn.SetColumn(2, tangent);
        tbn.SetColumn(1, bitangent);
        tbn.SetColumn(0, normal);

        for (int j = 0; j < profile_curve_verts.size(); ++j) {
            auto new_pos = tbn * profile_curve_verts[j] + guide_curve_verts[i];
            verticies.push_back(new_pos);
        }
        // Removed per-iteration face count; faces will be built after vertex
        // generation.
    }

    // Build face indices and face counts from the swept vertices.
    int profileCount = profile_curve_verts.size();
    int rings = guide_curve_verts.size();
    for (int i = 0; i < rings - 1; i++) {
        int ring1Index = i * profileCount;
        int ring2Index = (i + 1) * profileCount;
        for (int j = 0; j < profileCount; j++) {
            int next_j = (j + 1) % profileCount;
            face_vertex_indices.push_back(ring1Index + j);
            face_vertex_indices.push_back(ring2Index + j);
            face_vertex_indices.push_back(ring2Index + next_j);
            face_vertex_indices.push_back(ring1Index + next_j);
            face_vertex_counts.push_back(4);
        }
    }
    mesh->set_vertices(verticies);
    mesh->set_face_vertex_counts(face_vertex_counts);

    mesh->set_face_vertex_indices(face_vertex_indices);

    mesh->set_texcoords_array(texcoords_array);
    params.set_output("Mesh", mesh_geom);
    return true;
}

NODE_DECLARATION_UI(curve_to_mesh);
NODE_DEF_CLOSE_SCOPE
