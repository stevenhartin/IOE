////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

struct VPLSampleData_Struct
{
    float3 Position;
    float3 Normal;
    float4 Diffuse;
    float Roughness;
    float Intensity;
};

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
    float Intensity : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
// Buffers
////////////////////////////////////////////////////////////////////////////////

SamplerState g_PointSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

cbuffer GlobalSettings
{
    float4x4 ViewProj : WORLDVIEWPROJ;
}

cbuffer PassSettings
{
    float4 SampleDir;
    uint SampleIndex : SAMPLEINDEX;
}

StructuredBuffer<VPLSampleData_Struct> InVPLSampleData;

////////////////////////////////////////////////////////////////////////////////

#define PI 3.14159265f

////////////////////////////////////////////////////////////////////////////////

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

    VPLSampleData_Struct tStruct = InVPLSampleData[SampleIndex];
    float3 WorldPos = tStruct.Position;
    float OriginalFlux = tStruct.Intensity;

	float4x4 VPLXForm = float4x4(
		float4(1.0f, 0.0f, 0.0f, 0.0f),
		float4(0.0f, 1.0f, 0.0f, 0.0f),
		float4(0.0f, 0.0f, 1.0f, 0.0f),
		float4(-WorldPos, 1.0f));
    float4x4 WorldViewProjLight = mul(VPLXForm, ViewProj);

    Out.Pos = mul(float4(input.Pos.xyz, 1.0f), WorldViewProjLight);
    
    float3 ToLight = float3(WorldPos.xyz - input.Pos.xyz);
    // For a point light,
    // I' = I / (4 pi r^2)
    Out.Intensity = OriginalFlux / (dot(ToLight, ToLight) + 1.0f);

    return Out;
};