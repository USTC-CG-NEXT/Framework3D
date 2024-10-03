
#include <pybind11/pybind11.h>  // pybind11 protects this file from linking to debug library.
#include <torch/csrc/autograd/python_variable.h>

#include <boost/optional/optional_io.hpp>
#include <boost/python.hpp>
#include <boost/python/import.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/numpy/ndarray.hpp>

#include "../render/render_node_base.h"
#include "NODES_FILES_DIR.h"
#include "Nodes/node.hpp"
#include "Nodes/node_declare.hpp"
#include "Nodes/node_register.h"
#include "Nodes/socket_type_aliases.hpp"
#include "Nodes/socket_types/render_socket_types.hpp"
#include "Nodes/socket_types/stage_socket_types.hpp"
#include "entt/meta/resolve.hpp"
#include "func_node_base.h"
#include "pxr/base/vt/arrayPyBuffer.h"

namespace USTC_CG::node_python_script {
namespace bp = boost::python;
namespace bpn = boost::python::numpy;

#include <unordered_map>

static void add_input_according_to_typename(
    NodeDeclarationBuilder& b,
    const std::string& tname,
    const std::string& name){
    static const std::unordered_map<
        std::string,
        std::function<void(NodeDeclarationBuilder&, const std::string&)>>
        type_map = {
#define INSERT_INTO_MAP(tname_)                                           \
    {                                                                     \
        #tname_, [](NodeDeclarationBuilder& b, const std::string& name) { \
            b.add_input<decl::tname_>(name.c_str());                      \
        }                                                                 \
    }                                                                     \
    ,

            MACRO_MAP(INSERT_INTO_MAP, ALL_SOCKET_TYPES)
        };

auto it = type_map.find(tname);
if (it != type_map.end()) {
    it->second(b, name);
}
else {
    throw std::runtime_error("Unknown type name: " + tname);
}
}

static void add_output_according_to_typename(
    NodeDeclarationBuilder& b,
    const std::string& tname,
    const std::string& name){
    static const std::unordered_map<
        std::string,
        std::function<void(NodeDeclarationBuilder&, const std::string&)>>
        type_map = {
#define INSERT_INTO_MAP(tname_)                                           \
    {                                                                     \
        #tname_, [](NodeDeclarationBuilder& b, const std::string& name) { \
            b.add_output<decl::tname_>(name.c_str());                     \
        }                                                                 \
    }                                                                     \
    ,

            MACRO_MAP(INSERT_INTO_MAP, ALL_SOCKET_TYPES)
        };

auto it = type_map.find(tname);
if (it != type_map.end()) {
    it->second(b, name);
}
else {
    throw std::runtime_error("Unknown type name: " + tname);
}
}

#define DECLARE_PYTHON_SCRIPT(script)                                      \
    static void node_declare_##script(NodeDeclarationBuilder& b)           \
    {                                                                      \
        try {                                                              \
            bp::object module = bp::import(#script);                       \
            bp::object declare_node_info = module.attr("declare_node")();  \
            auto list = bp::list(declare_node_info);                       \
            auto input = bp::dict(list[0]);                                \
            auto output = bp::dict(list[1]);                               \
            for (int i = 0; i < len(input.keys()); ++i) {                  \
                auto key = input.keys()[i];                                \
                auto name = bp::extract<std::string>(key);                 \
                std::string tname = bp::extract<std::string>(input[key]);  \
                add_input_according_to_typename(b, tname, name);           \
            }                                                              \
            for (int i = 0; i < len(output); ++i) {                        \
                auto key = output.keys()[i];                               \
                auto name = bp::extract<std::string>(key);                 \
                std::string tname = bp::extract<std::string>(output[key]); \
                add_output_according_to_typename(b, tname, name);          \
            }                                                              \
        }                                                                  \
        catch (const bp::error_already_set&) {                             \
            PyErr_Print();                                                 \
            throw std::runtime_error(                                      \
                "Python error. Node delare fails in " #script);            \
        }                                                                  \
    };

//#define TryExraction(type)                                                    \
//    auto extract_##type##_array = pxr::VtArrayFromPyBuffer<type>(result);     \
//    if (extract_##type##_array.has_value()) {                                 \
//        params.set_output(object_name.c_str(), extract_##type##_array.get()); \
//        return;                                                               \
//    }
//
// #define ArrayTypes float, int, GfVec2f, GfVec3f, GfVec4f
//
// static void extract_value(
//    ExeParams& params,
//    const bp::object& result,
//    const std::string& object_name)
//{
//    using namespace pxr;
//
//    auto extract = bp::extract<float>(result);
//    if (extract.check()) {
//        float extract_float = extract;
//        params.set_output(object_name.c_str(), extract_float);
//        return;
//    }
//
//    MACRO_MAP(TryExraction, ArrayTypes);
//}

static void get_inputs(
    bp::list& input_l,
    const std::string& tname,
    const std::string& name,
    ExeParams& params)
{
    static const std::unordered_map<
        std::string,
        std::function<void(bp::list&, const std::string&, ExeParams&)>>
        type_map = {
#define INSERT_INTO_MAP(TYPE)                                                  \
    { #TYPE,                                                                   \
      [](bp::list& input_l, const std::string& name, ExeParams& params) {      \
          auto storage = params.get_input<socket_aliases::TYPE>(name.c_str()); \
          input_l.append(storage);                                             \
      } },
            MACRO_MAP(INSERT_INTO_MAP, ALL_SOCKET_TYPES_EXCEPT_SPECIAL)
        };

    auto it = type_map.find(tname);
    if (it != type_map.end()) {
        it->second(input_l, name, params);
    }
    else if (tname == "String") {
        auto storage =
            std::string(params.get_input<std::string>(name.c_str()).c_str());
        input_l.append(storage);
    }
    else if (tname == "PyObj") {
        auto storage = params.get_input<bp::object>(name.c_str());
        input_l.append(storage);
    }
    else if (tname == "TorchTensor") {
        auto storage = params.get_input<torch::Tensor>(name.c_str());
        PyObject* ptr = THPVariable_Wrap(storage);
        auto py_tensor = bp::object(bp::handle<>(bp::borrowed(ptr)));
        input_l.append(py_tensor);
        Py_DECREF(ptr);  // Clean up after borrowing
    }
    else {
        throw std::runtime_error("Unknown type name: " + tname);
    }
}

static void set_outputs(
    ExeParams& params,
    const bp::object& result,
    const std::string& tname,
    const std::string& name)
{
    static const std::unordered_map<
        std::string,
        std::function<void(ExeParams&, const bp::object&, const std::string&)>>
        type_map = {
#define INSERT_INTO_MAP(TYPE)                                                \
    { #TYPE,                                                                 \
      [](ExeParams& params,                                                  \
         const bp::object& result,                                           \
         const std::string& name) {                                          \
          auto value = bp::extract<socket_aliases::TYPE>(result);            \
          if (value.check()) {                                               \
              params.set_output(name.c_str(), socket_aliases::TYPE(value));  \
          }                                                                  \
          else {                                                             \
              throw std::runtime_error(                                      \
                  "Type not found for output, typename: " #TYPE ", name: " + \
                  name);                                                     \
          }                                                                  \
      } },

            MACRO_MAP(INSERT_INTO_MAP, ALL_SOCKET_TYPES_EXCEPT_SPECIAL)
        };

    auto it = type_map.find(tname);
    if (it != type_map.end()) {
        it->second(params, result, name);
    }
    else if (tname == "String") {
        auto value = bp::extract<std::string>(result);
        if (value.check()) {
            params.set_output(name.c_str(), std::string(value));
        }
        else {
            throw std::runtime_error(
                "Type not found for output, typename: String, name: " + name);
        }
    }
    else if (tname == "PyObj")  // If it is a pyobj, just set it.
    {
        auto value = result;
        params.set_output(name.c_str(), bp::object(value));
    }
    else if (tname == "TorchTensor") {
        static_assert(std::is_same_v<at::Tensor, torch::Tensor>);
        // If fails here, check whether you are really returning a torch tensor
        // following your declaration
        at::Tensor tensor = THPVariable_Unpack(result.ptr());
        params.set_output(name.c_str(), tensor);
    }
    else {
        throw std::runtime_error("Unknown type name: " + tname);
    }
}

constexpr bool ALWAYS_RELOAD = false;

#define DEFINE_PYTHON_SCRIPT_EXEC(script)                                     \
    static void node_exec_##script(ExeParams params)                          \
    {                                                                         \
        try {                                                                 \
            bp::object module = bp::import(#script);                          \
            bp::object declare_node_info = module.attr("declare_node")();     \
            auto list = bp::list(declare_node_info);                          \
            auto inputs = bp::dict(list[0]);                                  \
            auto outputs = bp::dict(list[1]);                                 \
            bp::list input_l;                                                 \
            for (int i = 0; i < len(inputs.keys()); ++i) {                    \
                auto tname =                                                  \
                    bp::extract<std::string>(inputs[inputs.keys()[i]]);       \
                auto name = bp::extract<std::string>(inputs.keys()[i]);       \
                get_inputs(input_l, tname, name, params);                     \
            }                                                                 \
            if constexpr (ALWAYS_RELOAD) {                                    \
                bp::object reload = bp::import("importlib").attr("reload");   \
                module = reload(module);                                      \
            }                                                                 \
            bp::object result = module.attr("wrap_exec")(input_l);            \
            if (len(outputs) > 1) {                                           \
                for (int i = 0; i < len(result); ++i) {                       \
                    auto tname =                                              \
                        bp::extract<std::string>(outputs[outputs.keys()[i]]); \
                    auto name = bp::extract<std::string>(outputs.keys()[i]);  \
                    set_outputs(params, result[i], tname, name);              \
                }                                                             \
            }                                                                 \
            else if (len(outputs) == 1) {                                     \
                auto tname =                                                  \
                    bp::extract<std::string>(outputs[outputs.keys()[0]]);     \
                auto name = bp::extract<std::string>(outputs.keys()[0]);      \
                set_outputs(params, result, tname, name);                     \
            }                                                                 \
        }                                                                     \
        catch (const bp::error_already_set&) {                                \
            PyErr_Print();                                                    \
            throw std::runtime_error("Python error.");                        \
        }                                                                     \
    }

#define BUILD_SCRIPT(script)      \
    DECLARE_PYTHON_SCRIPT(script) \
    DEFINE_PYTHON_SCRIPT_EXEC(script)

MACRO_MAP(BUILD_SCRIPT, FUNC_SCRIPT_LIST)
MACRO_MAP(BUILD_SCRIPT, RENDER_SCRIPT_LIST)

static void node_register()
{
#define REGISTER_FUNC_SCRIPT(script)                        \
    static NodeTypeInfo ntype##script;                      \
    strcpy(ntype##script.ui_name, #script);                 \
    strcpy(ntype##script.id_name, "python_script" #script); \
    func_node_type_base(&ntype##script);                    \
    ntype##script.node_execute = node_exec_##script;        \
    ntype##script.declare = node_declare_##script;          \
    nodeRegisterType(&ntype##script);

#define REGISTER_RENDER_SCRIPT(script)                      \
    static NodeTypeInfo ntype##script;                      \
    strcpy(ntype##script.ui_name, #script);                 \
    strcpy(ntype##script.id_name, "python_script" #script); \
    render_node_type_base(&ntype##script);                  \
    ntype##script.node_execute = node_exec_##script;        \
    ntype##script.declare = node_declare_##script;          \
    nodeRegisterType(&ntype##script);

    MACRO_MAP(REGISTER_FUNC_SCRIPT, FUNC_SCRIPT_LIST)
    MACRO_MAP(REGISTER_RENDER_SCRIPT, RENDER_SCRIPT_LIST)
}

NOD_REGISTER_NODE(node_register)
}  // namespace USTC_CG::node_python_script
