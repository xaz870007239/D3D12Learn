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

VSOut VS_Main(VertexData input)
{
    VSOut output;
    output.position = input.position;
    output.color = input.texcoord + color;
    return output;
}

float4 PS_Main(VSOut input) : SV_TARGET
{
    return input.color;
}
