
#include "nvrhi/nvrhi.h"
#include "entt/meta/resolve.hpp"
#include "pxr/base/vt/arrayPyBuffer.h"
#include "render_node_base.h"

#define LOAD_MODULE(name)                                       \
    bp::object module = bp::import(#name);                      \
    bp::object reload = bp::import("importlib").attr("reload"); \
    module = reload(module);

#include "nodes/core/def/node_def.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(rayleigh_sommerfeld)
{


}

NODE_EXECUTION_FUNCTION(rayleigh_sommerfeld)
{
    //using nparray = boost::python::numpy::ndarray;

    //auto arr = params.get_input<nparray>("Input");

    //namespace bp = boost::python;
    //try {
    //    LOAD_MODULE(rayleigh_sommerfeld)

    //    bp::object rst = module.attr("compute")(arr);
    //    bp::numpy::ndarray result = bp::numpy::array(rst);
    //    params.set_output("Output", result);
    //}
    //catch (const bp::error_already_set&) {
    //    PyErr_Print();
    //    log::error("Python error.")
    //}
    return true;
}



NODE_DECLARATION_UI(rayleigh_sommerfeld);
NODE_DEF_CLOSE_SCOPE
