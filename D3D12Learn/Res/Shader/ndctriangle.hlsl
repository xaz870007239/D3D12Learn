struct VertexData
{
    float4 position : POSITION;
    float4 texcoord : TEXCOORD0;
    float4 normal : NORMAL;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float4 color : TEXCOORD0;
};

VSOut VS_Main(VertexData input)
{
    VSOut output;
    output.position = input.position;
    output.color = input.position;
    //output.color = float4(1, 0, 0, 1);
    return output;
}

float4 PS_Main(VSOut input) : SV_TARGET
{
    return input.color;
}
