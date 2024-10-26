#pragma once
#include <entt/meta/factory.hpp>

USTC_CG_NAMESPACE_OPEN_SCOPE
template<typename TYPE>
void register_cpp_type()
{
    entt::meta<TYPE>().type(entt::type_hash<TYPE>());
    assert(entt::hashed_string{ typeid(TYPE).name() }, entt::type_hash<TYPE>());
}
USTC_CG_NAMESPACE_CLOSE_SCOPE