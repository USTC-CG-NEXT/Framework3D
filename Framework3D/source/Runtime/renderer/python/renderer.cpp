#include <nanobind/ndarray.h>
#include <nanobind/stl/map.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/vector.h>

#include "../nodes/glints/glints.hpp"
#include "nanobind/nanobind.h"
#include "nanobind/nb_defs.h"

namespace nb = nanobind;

NB_MODULE(hd_USTC_CG_py, m)
{
    nb::class_<USTC_CG::ScratchIntersectionContext>(
        m, "ScratchIntersectionContext")
        .def(nb::init<>())
        .def(
            "intersect_line_with_rays",
            [](USTC_CG::ScratchIntersectionContext& self,
               nb::ndarray<float> lines,
               nb::ndarray<float> patches,
               float width) {
                std::cout << "line_count " << lines.shape(0) << std::endl;
                std::cout << "patch_count " << patches.shape(0) << std::endl;
                auto [pairs, size] = self.intersect_line_with_rays(
                    lines.data(),
                    static_cast<unsigned>(lines.shape(0) / 2),
                    patches.data(),
                    patches.shape(0),
                    width);

                std::cout << "size: " << size << std::endl;

                return nb::ndarray<
                    nb::pytorch,
                    unsigned,
                    nb::ndim<2>,
                    nb::shape<-1, 2>,
                    nb::device::cuda>(pairs, { size, 2 });
            },
            nb::arg("lines"),
            nb::arg("patches"),
            nb::arg("width"),
            nb::rv_policy::reference)
        .def("reset", &USTC_CG::ScratchIntersectionContext::reset)
        .def(
            "set_max_pair_buffer_ratio",
            &USTC_CG::ScratchIntersectionContext::set_max_pair_buffer_ratio);

    nb::class_<USTC_CG::MeshIntersectionContext>(m, "MeshIntersectionContext")
        .def(nb::init<>())
        .def(
            "intersect_mesh_with_rays",
            [](USTC_CG::MeshIntersectionContext& self,
               nb::ndarray<float> vertices,
               nb::ndarray<float> indices,
               unsigned vertex_buffer_stride,
               const std::vector<int>& resolution,
               const std::vector<float>& world_to_clip) {
                std::cout << "vertices_count " << vertices.shape(0)
                          << std::endl;
                std::cout << "vertex_buffer_stride " << vertex_buffer_stride
                          << std::endl;
                auto [patches, corners, targets, count] =
                    self.intersect_mesh_with_rays(
                        vertices.data(),
                        static_cast<unsigned>(vertices.shape(0)),
                        vertex_buffer_stride,
                        indices.data(),
                        static_cast<unsigned>(indices.shape(0)),
                        int2(resolution[0], resolution[1]),
                        world_to_clip);

                std::cout << "count: " << count << std::endl;

                return std::make_tuple(
                    nb::ndarray<
                        nb::pytorch,
                        float,
                        nb::ndim<2>,
                        nb::shape<-1, sizeof(Patch) / sizeof(float)>,
                        nb::device::cuda>(
                        patches, { count, sizeof(Patch) / sizeof(float) }),
                    nb::ndarray<
                        nb::pytorch,
                        float,
                        nb::ndim<2>,
                        nb::shape<-1, sizeof(Corners) / sizeof(float)>,
                        nb::device::cuda>(
                        corners, { count, sizeof(Corners) / sizeof(float) }),
                    nb::ndarray<
                        nb::pytorch,
                        unsigned,
                        nb::ndim<2>,
                        nb::shape<-1, 2>,
                        nb::device::cuda>(targets, { count, 2 }),
                    count);
            },
            nb::arg("vertices"),
            nb::arg("indices"),
            nb::arg("vertex_buffer_stride"),
            nb::arg("resolution"),
            nb::arg("world_to_clip"),
            nb::rv_policy::reference);
}