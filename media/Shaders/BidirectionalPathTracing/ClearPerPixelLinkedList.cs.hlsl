#include "../common/common_defs.h"

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

struct FragmentData_Struct
{
    // RGB and Specular (stored in W component, defaults to 1)
    float4 Albedo;
    // Normal of fragment, could be stored as polar in uint32
    float4 Normal;
    // Radiance value for this fragment from the reflected fragment (if any at all)
    // Calculated in a second pass
    float4 Radiance;
    // 31 bit depth value, high value set to front / rear view.
    float Depth;
    // Roughness of fragment.
    float Roughness;
};

struct FragmentAndLinkBuffer_Struct
{
    FragmentData_Struct FragmentData;
    uint Next;
};

cbuffer ScreenSizes
{
    float ScreenWidth : SCREENWIDTH;
    float ScreenHeight : SCREENHEIGHT;
    int NumSamples : NUMSAMPLES;
};

static const int MaxElementsPerPixel = 8;

// Make sure to create with counter support D3D11_BUFFER_UAV_FLAG_COUNTER
RWStructuredBuffer<FragmentAndLinkBuffer_Struct> tRWFragmentAndLink : register(u0);

////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Entry
////////////////////////////////////////////////////////////////////////////////

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	for (int i=0; i < NumSamples; ++i)
	{
		[unroll]
		for (int j=0; j < MaxElementsPerPixel; ++j)
		{
			int Address = (i * MaxElementsPerPixel * ScreenWidth * ScreenHeight) + (DTid.x * ScreenWidth * MaxElementsPerPixel) + (DTid.y * MaxElementsPerPixel) + j;
		
		    tRWFragmentAndLink[Address].FragmentData.Albedo 	= float4(0.0f, 0.0f, 0.0f, 0.0f);
		    tRWFragmentAndLink[Address].FragmentData.Normal 	= float4(0.0f, 0.0f, 0.0f, 0.0f);
		    tRWFragmentAndLink[Address].FragmentData.Radiance 	= float4(0.0f, 0.0f, 0.0f, 0.0f);
		    tRWFragmentAndLink[Address].FragmentData.Depth 		= 0.0f;
		    tRWFragmentAndLink[Address].FragmentData.Roughness 	= 0.0f;
			tRWFragmentAndLink[Address].Next 					= 0xFFFFFFFF;
		}
	}
}