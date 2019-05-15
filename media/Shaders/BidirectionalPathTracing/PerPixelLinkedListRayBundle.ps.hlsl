// [See Yang et al 2010]

#include "../common/common_defs.h"
#include "../common/common.h"

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

// Make sure to create with counter support D3D11_BUFFER_UAV_FLAG_COUNTER
RWStructuredBuffer<FragmentAndLinkBuffer_Struct> tRWFragmentAndLink : register(u1);

// Screen-sized: width * height * sizeof(uint32), initialise to -1
// Head pointer buffer
RWByteAddressBuffer tRWFragmentListHead : register(u2);

cbuffer ScreenSizes
{
    float ScreenWidth : SCREENWIDTH;
    float ScreenHeight : SCREENHEIGHT;
};

cbuffer PassSettings
{
	uint CurrentSample;
}

#include "../common/common.h"

Texture2D DiffuseTexture;

SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};


struct PS_INPUT
{
    float4 Pos	    : SV_POSITION;
    float4 Normal   : NORMAL;
    float2 UV       : TEXCOORD0;      
    float Intensity : TEXCOORD1;
	float Depth		: TEXCOORD2;
    float Roughness : TEXCOORD3;
    float Specular : TEXCOORD4;
};

FragmentData_Struct ComputeFragment(PS_INPUT input, bool bIsFrontFace)
{
    FragmentData_Struct fragment = (FragmentData_Struct)0;
    //fragment.Albedo = float4(Diffuse_Lambert(DiffuseTexture.Sample(g_samPoint, input.UV).xyz), input.Specular);
    fragment.Albedo = float4(input.Specular,input.Specular,input.Specular,input.Specular);
    fragment.Normal = normalize(input.Normal);
    
    // Encode boolean in sign bit, -1.0f = bit it set (true)
    //fragment.Depth = max(DepthBufferValueToWorldDistance(input.Pos.z)) * (bIsFrontFace ? -1.0f : 1.0f);
	fragment.Depth = input.Depth * (bIsFrontFace ? -1.0f : 1.0f);
    
    //fragment.Depth = input.Pos.z;
    fragment.Roughness = input.Roughness;

    return fragment;
}

FragmentData_Struct FindFragment(const uint first, const float dir, const float depth)
{
    const float KeyDepth = dir * depth;
    uint index = first;
    FragmentData_Struct data;
    data.Depth = -FLT_MAX;
    
    // O(n) search through all fragments to find the 'matching' fragment. Much
    // quicker than keeping the entire list sorted through likes of depth peeling
    while (index != -1)
    {
        const FragmentAndLinkBuffer_Struct node = tRWFragmentAndLink[index];
        
        if (dir * node.FragmentData.Depth > 0.0f &&
            node.FragmentData.Depth > data.Depth &&
            node.FragmentData.Depth < KeyDepth)
        {
            data = node.FragmentData;
        }
        
        index = node.Next;
    }
    
    data.Depth = abs(data.Depth);
    
    return data;
}

uint GetAddress(int2 nScreenPos)
{
    uint SampleOffset = 4 * CurrentSample * (uint)ScreenWidth * (uint)ScreenHeight;
    uint RowOffset = 4 * (uint)ScreenWidth * (uint)nScreenPos.y;
    uint ColumnOffset = 4 * (uint)nScreenPos.x;
    return ColumnOffset + RowOffset + SampleOffset;
}

// NO RENDER TARGET BOUND!!!!!
//[earlydepthstencil]
float4 main( PS_INPUT input, bool bIsFrontFace : SV_IsFrontFace ) : SV_TARGET0
{
    FragmentData_Struct tFragmentData = ComputeFragment(input, bIsFrontFace);

    // Write the fragment attributes to the node buffer, ensuring the reverse
    // link is stored too.
    FragmentAndLinkBuffer_Struct tLinkBuffer;
    tLinkBuffer.FragmentData =tFragmentData;

    // Get current pixel counter value and increment the counter on
    uint uNewFragmentAddress = tRWFragmentAndLink.IncrementCounter();

    int2 vScreenAddress = int2(input.Pos.xy);
    uint uStartAddress = GetAddress(vScreenAddress);
    
    // Store the new link at uStartAddress and return the existing value
    // in uOldAddress. If this is the first link, then uOldAddress will be -1
    // (what the fragment list head is initialised to), otherwise, it's whatever
    // was rendered there last.
    uint nOldStartOffset=0xFFFFFFFF;
    tRWFragmentListHead.InterlockedExchange(
        uStartAddress, uNewFragmentAddress, nOldStartOffset);
    

    tLinkBuffer.Next = nOldStartOffset;
    tRWFragmentAndLink[uNewFragmentAddress] = tLinkBuffer;

    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}