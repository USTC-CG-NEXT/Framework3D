#pragma once

#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

class IWidget {
public:
    virtual ~IWidget() = default;
    virtual void BuildUI() = 0;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
