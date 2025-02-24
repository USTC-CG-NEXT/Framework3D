//#include <nanobind/nanobind.h>
//
//#include <RHI/rhi.hpp>
//
//namespace nb = nanobind;
//
//using namespace USTC_CG;
//
//NB_MODULE(RHI_py, m)
//{
//    m.def(
//        "init",
//        &RHI::init,
//        nb::arg("with_window") = false,
//        nb::arg("use_dx12") = false);
//    m.def("shutdown", &RHI::shutdown);
//    m.def("get_device", &RHI::get_device, nb::rv_policy::reference);
//    m.def("get_backend", &RHI::get_backend);
//    m.def(
//        "calculate_bytes_per_pixel",
//        &RHI::calculate_bytes_per_pixel,
//        nb::arg("format"));
//    m.def("load_texture", &RHI::load_texture, nb::arg("desc"), nb::arg("data"));
//    m.def(
//        "write_texture",
//        &RHI::write_texture,
//        nb::arg("texture"),
//        nb::arg("staging"),
//        nb::arg("data"));
//    m.def(
//        "load_ogl_texture",
//        &RHI::load_ogl_texture,
//        nb::arg("desc"),
//        nb::arg("gl_texture"));
//}