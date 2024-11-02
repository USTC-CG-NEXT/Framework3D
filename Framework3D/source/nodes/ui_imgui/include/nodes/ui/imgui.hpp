#pragma once
#include <memory>

#include "GUI/widget.h"

#include "nodes/ui/api.h"
USTC_CG_NAMESPACE_OPEN_SCOPE
class NodeTree;
std::unique_ptr<IWidget> create_node_imgui_widget(NodeTree* tree);

USTC_CG_NAMESPACE_CLOSE_SCOPE