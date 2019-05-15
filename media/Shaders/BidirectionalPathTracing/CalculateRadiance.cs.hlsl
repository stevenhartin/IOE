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

struct FragmentRadiance_Struct
{
	float4 Radiance;
};

cbuffer RarelyModified
{
    float ScreenWidth : SCREENWIDTH;
    float ScreenHeight : SCREENHEIGHT;
};

cbuffer Globals
{
	float4 RayDir : DIRECTION;
	uint CurrentSample : CURRENTSAMPLE;
};

////////////////////////////////////////////////////////////////////////////////
// Buffers
////////////////////////////////////////////////////////////////////////////////

// Input fragment data
RWStructuredBuffer<FragmentAndLinkBuffer_Struct> InOutFragmentAndLink : register(u0);

// Input head pointer data
ByteAddressBuffer InFragmentListHead : register(u1);

////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////

FragmentData_Struct FindFragment(const uint first, const float dir, const float depth)
{
    const float KeyDepth = dir * depth;
    uint index = first;
    FragmentData_Struct data;
    data.Depth = FLT_MAX;
    
    // O(n) search through all fragments to find the 'matching' fragment. Much
    // quicker than keeping the entire list sorted through likes of depth peeling
    while (index != -1)
    {
        const FragmentAndLinkBuffer_Struct node = InOutFragmentAndLink[index];
        
        if (dir * node.FragmentData.Depth > 0.0f &&
            node.FragmentData.Depth < data.Depth &&
            node.FragmentData.Depth > KeyDepth)
        {
            data = node.FragmentData;
        }
        
        index = node.Next;
    }
    
    data.Depth = abs(data.Depth);
    
    return data;
}

////////////////////////////////////////////////////////////////////////////////

float4 GetRadiance(FragmentData_Struct first, FragmentData_Struct second)
{
	// Only transfer radiance if the fragments are facing each other
    float4 ReturnRadiance = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (dot(first.Normal, second.Normal) < 0.0f)
	{
		// For point lights, radiance transfer is just 1/dist^2
		float fDist = abs(first.Depth - second.Depth);

		// We multiply the radiance by the albedo to obtain our spectrum
		// values
		ReturnRadiance = second.Albedo * (1.0f / (fDist * fDist));
	}
	return ReturnRadiance;
}

////////////////////////////////////////////////////////////////////////////////

int GetAddress(int2 nScreenPos)
{
    int offset = CurrentSample * (int)ScreenWidth * (int)ScreenHeight * 4;
    return (int)(4 * ((ScreenWidth * nScreenPos.y) + nScreenPos.x) + offset);
}

////////////////////////////////////////////////////////////////////////////////
// Entry
////////////////////////////////////////////////////////////////////////////////

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	int Address = GetAddress(DTid.xy);

	// Pull the first index in the list, this is our starting fragment. Loop
	// Over all fragments within this pixel and transfer any radiance.
	uint ThisIndex = InFragmentListHead.Load(Address);

	while (ThisIndex != -1)
	{
		// Grab the fragment data for this pixel coord
		FragmentAndLinkBuffer_Struct ThisFragmentAndLink = InOutFragmentAndLink[ThisIndex];
		FragmentData_Struct ThisFragment = ThisFragmentAndLink.FragmentData;

		// If the normal to the surface is in the direction of the global ray direction, then we need to find
		// The next vertex in the forward direction, otherwise reverse search.
		float SearchDirection = dot(ThisFragment.Normal.xyz, RayDir.xyz) > 0.0f ? 1.0f : -1.0f;

		// Find the fragment which is facing this fragment. If we found one, transfer the radiance.
		FragmentData_Struct ReflectedFragment = FindFragment(ThisIndex, SearchDirection, ThisFragment.Depth);
		if (ReflectedFragment.Depth < FLT_MAX)
		{
			// Found matching fragment data, computer the radiance transfer
			InOutFragmentAndLink[ThisIndex].FragmentData.Radiance = GetRadiance(ThisFragment, ReflectedFragment);
		}
		else
		{
			// No matching fragment data, store 0 radiance
			InOutFragmentAndLink[ThisIndex].FragmentData.Radiance = float4(0.0f, 0.0f, 0.0f, 1.0f);

		}
		// Carry on to the next fragment
		ThisIndex = ThisFragmentAndLink.Next;
	}
}