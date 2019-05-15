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
    float4 ProjectedPos : SV_POSITION;
    float Intensity     : TEXCOORD0;
    float ClipDepth     : TEXCOORD1;	
    float Depth         : TEXCOORD2;
	float3 ToLight		: NORMAL;		
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
    float4x4 ViewProj   : WORLDVIEWPROJ;
}

cbuffer PassSettings
{
    float Far;
    float Near;
    uint SampleIndex : SAMPLEINDEX;

    float4 SampleDir;
}

StructuredBuffer<VPLSampleData_Struct> InVPLSampleData;

////////////////////////////////////////////////////////////////////////////////

#include "../common/common_defs.h"

////////////////////////////////////////////////////////////////////////////////

VS_OUTPUT main(VS_INPUT In)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

    VPLSampleData_Struct tStruct = InVPLSampleData[SampleIndex];
    float3 WorldPos = tStruct.Position;
    float3 WorldNormal = tStruct.Normal;
    float OriginalFlux = tStruct.Intensity;

    float3 UpDirection = float3(0.0f, 1.0f, 0.0f);
    // Assumes WorldNormal is normalised to avoid division
    // float ScalarProduct = dot(WorldNormal, UpDirection) / (length(WorldNormal) * length(UpDirection))
    //                     = WorldNormal.UpDirection / (1 * 1)
    //                     = WorldNormal.UpDirection / 1
    //                     = WolrdNormal.UpDirection
    float ScalarProduct = dot(WorldNormal, UpDirection);
    if (abs(ScalarProduct) >= 1.0f - Tolerance)
    {
        UpDirection = float3(0.0f, 0.0f, -1.0f);
    }

    float4x4 VPLView = CreateLookAtLH(float3(0.0f, 0.0f, 0.0f), WorldNormal, 
        UpDirection);

	float4x4 VPLXForm = float4x4(
		float4(1.0f, 0.0f, 0.0f, 0.0f),
		float4(0.0f, 1.0f, 0.0f, 0.0f),
		float4(0.0f, 0.0f, 1.0f, 0.0f),
		float4(-WorldPos, 1.0f));
    float4x4 WorldViewProjLight = mul(VPLXForm, VPLView);
    
    // transform vertex to DP-space
	Out.ProjectedPos = mul(float4(In.Pos.xyz, 1.0f), WorldViewProjLight);
    Out.ProjectedPos /= Out.ProjectedPos.w;
    
    float fLength = length(Out.ProjectedPos.xyz);
    
    // because the origin is at 0 the proj-vector
    // matches the vertex-position
    
    // normalize
    Out.ProjectedPos /= fLength;
        
    // save for clipping 	
    Out.ClipDepth = Out.ProjectedPos.z;
    
    // calc "normal" on intersection, by adding the 
    // reflection-vector(0,0,1) and divide through 
    // his z to get the texture coords
    Out.ProjectedPos.x /= Out.ProjectedPos.z + 1.0f;
    Out.ProjectedPos.y /= Out.ProjectedPos.z + 1.0f;
    
    // set z for z-buffering and neutralize w
    Out.ProjectedPos.z = ((fLength - Near) / (Far - Near));
    Out.ProjectedPos.w = 1.0f;
    
    // DP-depth
    Out.Depth = Out.ProjectedPos.z;
    
    float3 ToLight = float3(WorldPos.xyz - In.Pos.xyz);
    
    Out.Intensity = OriginalFlux;
    Out.ToLight = ToLight;

    return Out;
};