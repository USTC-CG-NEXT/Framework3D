#include <iostream>

#include "Logger/Logger.h"
#include "nodes/system/node_system.hpp"

int main()
{
    using namespace USTC_CG;

    log::SetMinSeverity(Severity::Info);
    log::EnableOutputToConsole(true);



    std::unique_ptr<NodeSystem> system_ = create_dynamic_loading_system();
    system_->register_cpp_types<int>();

    auto loaded = system_->load_configuration("test_nodes.json");

    system_->init();



}