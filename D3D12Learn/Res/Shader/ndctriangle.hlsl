struct VertexData
{
    float4 position : POSITION;
    float4 texcoord : TEXCOORD0;
    float4 normal   : NORMAL;
    float4 tangent  : TANGENT;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float4 normal   : NORMAL;
};

static const float PI = 3.141592;

cbuffer GlobalConstants : register(b0)
{
    float4 color;
};

cbuffer DefaultVertexCB : register(b1)
{
    float4x4 ProjectionMatrix;
    float4x4 ViewMatrix;
    float4x4 ModelMatrix;
    float4x4 ITMatrix;
    float4x4 ReservedMemory[1020];
};

VSOut VS_Main(VertexData input)
{
    VSOut output;
    float4 positionWS = mul(ModelMatrix, input.position);
    float4 positionVS = mul(ViewMatrix, positionWS);
    output.position = mul(ProjectionMatrix, positionVS);
    output.normal = input.normal;
    return output;
}

float4 PS_Main(VSOut input) : SV_TARGET
{
    float3 N = normalize(input.normal.xyz);
    float3 bottomColor = float3(0.1f, 0.4f, 0.6f);
    float3 topColor = float3(0.7f, 0.7f, 0.7f);
    
    float theta = asin(N.y);
    theta /= PI;
    theta += 0.5f;

    float3 ambientColor = lerp(bottomColor, topColor, theta);
    float3 diffuseColor = float3(0.0f, 0.0f, 0.0f);
    float3 specularColor = float3(0.0f, 0.0f, 0.0f);
    float3 surfaceColor = ambientColor + diffuseColor + specularColor;
    return float4(surfaceColor, 1.0f);
}
