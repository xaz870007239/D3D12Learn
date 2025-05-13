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
    float4 color : TEXCOORD0;
};

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
    output.color = input.texcoord + color;
    return output;
}

float4 PS_Main(VSOut input) : SV_TARGET
{
    return input.color;
}
