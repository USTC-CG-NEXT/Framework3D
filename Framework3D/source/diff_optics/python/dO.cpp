#include "diff_optics/lens_system_compiler.hpp"
#include "nanobind/nanobind.h"

NB_MODULE(diff_optics_py, m)
{
    using namespace nanobind;
    using namespace USTC_CG;

    //class_<LensSystem>(m, "LensSystem")
    //    .def(nanobind::init<>())
    //    .def("add_lens", &LensSystem::add_lens)
    //    .def("lens_count", &LensSystem::lens_count)
    //    .def(
    //        "deserialize",
    //        overload_cast<const std::string&>(&LensSystem::deserialize))
    //    .def(
    //        "deserialize",
    //        overload_cast<const std::filesystem::path&>(
    //            &LensSystem::deserialize))
    //    .def("set_default", &LensSystem::set_default);

    //class_<LensLayer>(m, "LensLayer")
    //    .def("set_axis", &LensLayer::set_axis)
    //    .def("set_pos", &LensLayer::set_pos)
    //    .def("deserialize", &LensLayer::deserialize)
    //    .def("fill_block_data", &LensLayer::fill_block_data);

    //class_<NullLayer, LensLayer>(m, "NullLayer")
    //    .def(nanobind::init<float, float>());

    //class_<Occluder, LensLayer>(m, "Occluder")
    //    .def(nanobind::init<float, float, float>())
    //    .def("deserialize", &Occluder::deserialize)
    //    .def("fill_block_data", &Occluder::fill_block_data);

    //class_<SphericalLens, LensLayer>(m, "SphericalLens")
    //    .def(nanobind::init<float, float, float, float>())
    //    .def("deserialize", &SphericalLens::deserialize)
    //    .def("fill_block_data", &SphericalLens::fill_block_data);

    //class_<FlatLens, LensLayer>(m, "FlatLens")
    //    .def(nanobind::init<float, float, float>())
    //    .def("deserialize", &FlatLens::deserialize)
    //    .def("fill_block_data", &FlatLens::fill_block_data);

    class_<LensSystemCompiler>(m, "LensSystemCompiler")
        .def(nanobind::init<>())
        .def("emit_line", &LensSystemCompiler::emit_line)
        .def("compile", &LensSystemCompiler::compile)
        .def_static("fill_block_data", &LensSystemCompiler::fill_block_data);

    class_<CompiledDataBlock>(m, "CompiledDataBlock")
        .def(nanobind::init<>())
        .def_rw("parameters", &CompiledDataBlock::parameters)
        .def_rw("parameter_offsets", &CompiledDataBlock::parameter_offsets)
        .def_rw("cb_size", &CompiledDataBlock::cb_size);
}