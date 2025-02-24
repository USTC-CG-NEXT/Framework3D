// Random number generation using XOR-Shift algorithm
#include "random.slangh"

[[vk::binding(0, 0)]] RWTexture2D<uint> inout_random_init : register(u0);

[numthreads(16, 16, 1)]  // Number of threads in a group
void main(uint3 DTid: SV_DispatchThreadID)
{
    uint2 coord = DTid.xy;
    uint seed = (coord.x + 1) * (coord.y + 1) * 2654435761u; // Better seed calculation using a prime number
    uint randomValue = random_unsigned(seed);
    inout_random_init[coord] = randomValue;
}
