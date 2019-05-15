struct VS_INPUT
{
    float4 Pos      : POSITION;
    float3 UV	    : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos		: SV_POSITION;
    float3 UV		: TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;
    Out.Pos = input.Pos;
    Out.UV = input.UV;

    return Out;
}
