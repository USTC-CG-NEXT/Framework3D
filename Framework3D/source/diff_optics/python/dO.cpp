#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/map.h>
#include <nanobind/stl/shared_ptr.h>

#include "diff_optics/lens_system_compiler.hpp"
#include "nanobind/nanobind.h"

NB_MODULE(diff_optics_py, m)
{
    using namespace nanobind;
    using namespace USTC_CG;

    class_<LensSystem>(m, "LensSystem")
        .def(nanobind::init<>())
        .def("add_lens", &LensSystem::add_lens)
        .def("lens_count", &LensSystem::lens_count)
        .def(
            "deserialize",
            overload_cast<const std::string&>(&LensSystem::deserialize))
        .def(
            "deserialize",
            overload_cast<const std::filesystem::path&>(
                &LensSystem::deserialize))
        .def("set_default", &LensSystem::set_default);

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