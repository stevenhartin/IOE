#include "../common/common_defs.h"

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

cbuffer Globals
{
	float4 RayDir : DIRECTION;
	uint CurrentSample : CURRENTSAMPLE;
};

SamplerState PointSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

////////////////////////////////////////////////////////////////////////////////
// Buffers
////////////////////////////////////////////////////////////////////////////////

// ALl the buffers required for the reflective shadow map. This should probably
// be remade as a TextureCubeArray instead of a TextureCube to be more
// performant and memory conservative.
TextureCube InReflectiveShadowMap_Position;
TextureCube InReflectiveShadowMap_Normal;
TextureCube InReflectiveShadowMap_Intensity;
TextureCube InReflectiveShadowMap_Diffuse;
TextureCube InReflectiveShadowMap_Roughness;

// Output for the VPL Sample data
RWStructuredBuffer<VPLSampleData_Struct> OutVPLSampleData;

////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Entry
////////////////////////////////////////////////////////////////////////////////

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // TODO: Make the address the thread ID. As it stands, we have to call
    // dispatch multiple times overriding the cbuffer as we don't have a buffer
    // of all the sample dirs
	//int Address = DTid.x;
    //float3 RayDir = InSamplerDirsBuffer[Address];

    VPLSampleData_Struct tStruct;

	tStruct.Normal = InReflectiveShadowMap_Normal.SampleLevel(PointSampler, RayDir.xyz, 0).xyz;
    tStruct.Position = InReflectiveShadowMap_Position.SampleLevel(PointSampler, RayDir.xyz, 0).xyz/* + tStruct.Normal*/;
	tStruct.Diffuse = InReflectiveShadowMap_Diffuse.SampleLevel(PointSampler, RayDir.xyz, 0);
	tStruct.Roughness = InReflectiveShadowMap_Roughness.SampleLevel(PointSampler, RayDir.xyz, 0).r;
	tStruct.Intensity = InReflectiveShadowMap_Intensity.SampleLevel(PointSampler, RayDir.xyz, 0).r;

	//OutVPLSampleData[Address] = tStruct;
    OutVPLSampleData[CurrentSample] = tStruct;
}