#include <map>

#include "Logger/Logger.h"
#include "nodes/core/api.h"
#include "entt/meta/resolve.hpp"
#include "nodes/core/node.hpp"
#include "node_register.h"
#include "nodes/core/node.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

static std::map<std::string, NodeTypeInfo*> geo_node_registry;

const std::map<std::string, NodeTypeInfo*>& get_geo_node_registry()
{
    return geo_node_registry;
}

static std::map<std::string, NodeTypeInfo*> render_node_registry;

const std::map<std::string, NodeTypeInfo*>& get_render_node_registry()
{
    return render_node_registry;
}

static std::map<std::string, NodeTypeInfo*> func_node_registry;

const std::map<std::string, NodeTypeInfo*>& get_func_node_registry()
{
    return func_node_registry;
}

static std::map<std::string, NodeTypeInfo*> composition_node_registry;

const std::map<std::string, NodeTypeInfo*>& get_composition_node_registry()
{
    return composition_node_registry;
}

std::map<std::string, NodeTypeInfo*> conversion_node_registry;

const std::map<std::string, NodeTypeInfo*>& get_conversion_node_registry()
{
    return conversion_node_registry;
}

void register_all()
{
    //Py_Initialize();
    //// Call python to set python path
    //namespace py = boost::python;
    //py::object sys = py::import("sys");
    //sys.attr("path").attr("append")(FUNC_NODES_FILES_DIR "/scripts");
    //sys.attr("path").attr("append")(RENDER_NODES_FILES_DIR "/scripts");

    //boost::python::numpy::initialize();
    //register_cpp_types();
    //register_nodes();
    //register_sockets();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
