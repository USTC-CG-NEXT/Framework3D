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
            [](USTC_CG::ScratchIntersectionContext &self,
               nb::ndarray<float> lines,
               nb::ndarray<float> patches,
               float width) {
                std::cout << "line_count " << lines.shape(0) << std::endl;
                std::cout << "patch_count " << patches.shape(0) << std::endl;
                auto [pairs, size] = self.intersect_line_with_rays(
                    lines.data(),
                    lines.shape(0) / 2,
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
            [](USTC_CG::MeshIntersectionContext &self,
               nb::ndarray<float> vertices,
               nb::ndarray<float> indices,
               nb::ndarray<float> rays) {
                auto vertex_count = vertices.shape(0);
                auto index_count = indices.shape(0);
                auto vertex_stride = vertices.shape(1);

                auto [pairs, targets, count] = self.intersect_mesh_with_rays(
                    vertices.data(),
                    vertex_count,
                    vertex_stride,
                    indices.data(),
                    index_count,
                    rays.data(),
                    rays.shape(0));

                std::cout << "count: " << count << std::endl;
                // return 2 arrays
                return std::make_tuple(
                    nb::ndarray<
                        nb::pytorch,
                        unsigned,
                        nb::ndim<2>,
                        nb::shape<-1, 2>,
                        nb::device::cuda>(pairs, { count, 2 }),
                    nb::ndarray<
                        nb::pytorch,
                        unsigned,
                        nb::ndim<1>,
                        nb::shape<-1>,
                        nb::device::cuda>(targets, { count }));
            },
            nb::arg("vertices"),
            nb::arg("indices"),
            nb::arg("rays"),
            nb::rv_policy::reference);
}