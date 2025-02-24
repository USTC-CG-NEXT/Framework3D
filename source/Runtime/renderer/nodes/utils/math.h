#pragma once
namespace USTC_CG {

inline int div_ceil(int dividend, int divisor)
{
    return (dividend + (divisor - 1)) / divisor;
}

}  // namespace USTC_CG