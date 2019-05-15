cbuffer GlobalSettings
{
    float4x4 WorldViewProj : WORLDVIEWPROJ;
	float4 View : VIEW;
}

cbuffer RarelyModified
{
	float4 LightPosition;
	float LightIntensity;
    float Roughness : ROUGHNESS;
    float Specular : SPECULAR;
}

struct VS_INPUT
{
    float4 Pos      : POSITION;
    float4 Nrm      : NORMAL;
    float4 Colour   : COLOR;
    float2 UV	    : TEXCOORD0;
    float4 Tangent  : TEXCOORD1;
    float4 Binormal : TEXCOORD2;
};

struct VS_OUTPUT
{
    float4 Pos		: SV_POSITION;
    float4 Normal	: NORMAL;
    float2 UV       : TEXCOORD0;
    float Intensity : TEXCOORD1;
	float Depth		: TEXCOORD2;
    float Roughness : TEXCOORD3;
    float Specular  : TEXCOORD4;
};

#define PI 3.14159265f

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

    Out.Pos = mul(float4(input.Pos.xyz, 1.0f), WorldViewProj);
    Out.Normal = input.Nrm;
    Out.UV = input.UV;
	float3 ToLight = LightPosition.xyz - input.Pos.xyz;
    Out.Intensity = /*LightIntensity / (dot(ToLight, ToLight) + 1.0f)*/0.1f;
	Out.Depth = length(View.xyz - input.Pos.xyz);
    Out.Roughness = Roughness;
    Out.Specular = Specular; 

    return Out;
};