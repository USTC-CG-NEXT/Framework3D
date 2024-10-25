#pragma once
#include <entt/meta/factory.hpp>

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace nodes {
template<typename TYPE>
void register_cpp_type()
{
    entt::meta<TYPE>().type(entt::hashed_string{ typeid(TYPE).name() });
}
}  // namespace nodes
USTC_CG_NAMESPACE_CLOSE_SCOPE