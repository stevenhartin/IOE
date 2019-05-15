// input
struct VSInput
{
	float4 Pos : POSITION;
	float3 UV : TEXCOORD0;
};

// output
struct VSOutput
{
	float4 Pos: SV_POSITION;
	float2 UV : TEXCOORD0;
};

cbuffer Globals
{
	float4x4 WorldViewProj;
};

VSOutput main(VSInput input)
{
	VSOutput Output = (VSOutput)0;
	Output.Pos = mul(float4(input.Pos.xyz, 1.0f), WorldViewProj);
	Output.UV = input.UV.xy;
	return Output;
}
