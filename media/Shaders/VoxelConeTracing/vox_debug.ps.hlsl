struct PS_INPUT
{
    float4 Pos		: SV_POSITION;
    float3 UV		: TEXCOORD0;
    float3 offset	: TEXCOORD1;
};

Texture3D sampler0; // +X
Texture3D sampler1; // -X
Texture3D sampler2; // +Y
Texture3D sampler3; // -Y
Texture3D sampler4; // +Z
Texture3D sampler5; // -Z

SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

cbuffer Test
{
	float lod;
}

// dir the direction from the center of the voxel outward
// dir should be normalized
float4 voxelFetch(float3 pos, float3 Normal)
{
	float4 sampleX =
		Normal.x < 0.0
		? sampler0.SampleLevel(g_samPoint, pos, lod)
		: sampler1.SampleLevel(g_samPoint, pos, lod);
	
	float4 sampleY =
		Normal.y < 0.0
		? sampler2.SampleLevel(g_samPoint, pos, lod)
		: sampler3.SampleLevel(g_samPoint, pos, lod);
	
	float4 sampleZ =
		Normal.z < 0.0
		? sampler4.SampleLevel(g_samPoint, pos, lod)
		: sampler5.SampleLevel(g_samPoint, pos, lod);
	
	float3 sampleWeights = abs(Normal);
	// ignore empty samples?
	// makes sense in the debug view, but probably not when actually tracing
	if (sampleX.w == 0.0)
		sampleWeights.x = 0.0;
	if (sampleY.w == 0.0)
		sampleWeights.y = 0.0;
	if (sampleZ.w == 0.0)
		sampleWeights.z = 0.0;
	
	float invSampleMag = 1.0 / (dot(sampleWeights, float3(1.0f, 1.0f, 1.0f)) + .0001);
	
	float4 filtered = 
		sampleX * sampleWeights.x
		+ sampleY * sampleWeights.y
		+ sampleZ * sampleWeights.z;
	filtered *= invSampleMag;

	return filtered;
    //return sampleX;
}

float4 main(PS_INPUT input) : SV_TARGET0
{
	float4 diffuse = voxelFetch(input.UV, normalize(input.offset));
	
	clip(diffuse.a - 0.01f);
	
	diffuse.a = 1.0f;
	return diffuse;
}
