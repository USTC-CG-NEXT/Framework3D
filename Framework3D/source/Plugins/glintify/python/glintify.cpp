#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>

#include <glintify/glintify.hpp>

namespace nb = nanobind;

NB_MODULE(glintify_py, m)
{
    nb::class_<USTC_CG::StrokeSystem>(m, "StrokeSystem")
        .def(nb::init<>())
        .def("get_all_endpoints", &USTC_CG::StrokeSystem::get_all_endpoints)
        .def(
            "set_camera_move_range",
            &USTC_CG::StrokeSystem::set_camera_move_range)
        .def("set_camera_position", &USTC_CG::StrokeSystem::set_camera_position)
        .def("set_light_position", &USTC_CG::StrokeSystem::set_light_position)
        .def("set_occlusion", &USTC_CG::StrokeSystem::set_occlusion)
        .def("calc_scratches", &USTC_CG::StrokeSystem::calc_scratches)
        .def("add_virtual_point", &USTC_CG::StrokeSystem::add_virtual_point)
        .def("clear", &USTC_CG::StrokeSystem::clear)
        .def(
            "fill_ranges",
            &USTC_CG::StrokeSystem::fill_ranges,
            nb::arg("consider_occlusion") = false);
}