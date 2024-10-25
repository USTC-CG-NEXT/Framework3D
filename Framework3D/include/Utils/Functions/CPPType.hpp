#pragma once

#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

template<typename TYPE>
void register_cpp_type()
{
    entt::meta<TYPE>().type(entt::hashed_string{ typeid(TYPE).name() });
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
