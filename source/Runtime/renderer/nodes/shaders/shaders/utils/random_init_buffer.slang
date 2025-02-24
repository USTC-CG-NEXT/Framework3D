// Random number generation using XOR-Shift algorithm
#include "random.slangh"

[[vk::binding(0, 0)]]
RWStructuredBuffer<uint> inout_random : register(u0);

[numthreads(128, 1, 1)]  // Number of threads in a group
void main(uint3 DTid: SV_DispatchThreadID)
{
    uint coord = DTid.x;
    uint seed =
        (coord.x + 1) * 2654435761u;  // Better seed calculation using a prime number
    uint randomValue = random_unsigned(seed);
    inout_random[coord] = randomValue;
}
