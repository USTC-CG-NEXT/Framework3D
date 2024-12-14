// Random number generation using XOR-Shift algorithm
#include "random.slangh"

RWStructuredBuffer<uint> inout_random;

[numthreads(128, 1, 1)]  // Number of threads in a group
void main(uint3 DTid: SV_DispatchThreadID)
{
    uint coord = DTid.x;
    var seed = inout_random[coord];
    uint randomValue = random_unsigned(seed);
    inout_random[coord] = randomValue;
}
