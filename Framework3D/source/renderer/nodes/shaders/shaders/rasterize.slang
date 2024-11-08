// Draw instanced geometry to a render target.
struct MyStruct
{
    int i = 0;
    int j = 3;
}
// Shader bindings
cbuffer PerFrameConstants : register(b0) {
    float4x4 viewProjectionMatrix;
    float3 lightDirection;
}


// Using a StructuredBuffer to hold the model matrices
StructuredBuffer<float4x4> modelMatrixBuffer : register(t0);

// Function to get the model matrix for a given instance
float4x4 GetModelMatrix(uint instanceID) {
    return modelMatrixBuffer[instanceID];
}

// Vertex shader
struct VSOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

VSOutput vs_main(float3 position : POSITION, float3 normal : NORMAL, uint instanceID : SV_InstanceID) {
    VSOutput output;
    float4x4 modelMatrix = GetModelMatrix(instanceID);
    output.position = mul(modelMatrix, float4(position, 1.0));
    output.normal = mul((float3x3)modelMatrix, normal);
    return output;
}

// Pixel shader
float4 ps_main(VSOutput input) : SV_TARGET {
    float3 lightDir = normalize(float3(0.5, 0.5, -1.0));
    float3 normal = normalize(input.normal);
    float diffuse = max(dot(normal, lightDir), 0.0);
    return float4(diffuse, diffuse, diffuse, 1.0);
}

// Shader entry points
VSOutput vs_main(float3 position : POSITION, float3 normal : NORMAL, uint instanceID : SV_InstanceID);
float4 ps_main(VSOutput input) : SV_TARGET;