#pragma once

#include <memory>
#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class NodeTree;

namespace nodes {

std::unique_ptr<NodeTree> create_node_tree();

namespace io {

}

}  // namespace nodes

USTC_CG_NAMESPACE_CLOSE_SCOPE