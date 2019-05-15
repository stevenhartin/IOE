//////////////////////////////////////////////////////////////////////////
// Structures
//////////////////////////////////////////////////////////////////////////

#include "../common/common_defs.h"

//////////////////////////////////////////////////////////////////////////

// Input
struct GSInput
{
	float4 Pos : SV_POSITION;
};

// Output
struct GSOutput
{
	float4 Pos: SV_POSITION;
    float4 Colour : TEXCOORD0;
};

// CBuffer
cbuffer Globals
{
	float4x4 WorldViewProj;
    float4x4 RayBundleXFormInv;
    float4 RayBundleOrigin;
    float4 RayBundleDirection;
    int SampleIndex;
    float ScreenWidth;
    float ScreenHeight;
};

// Fragment data
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

// Fragment pixel linked list
struct FragmentAndLinkBuffer_Struct
{
    FragmentData_Struct FragmentData;
    uint Next;
};

//////////////////////////////////////////////////////////////////////////

StructuredBuffer<FragmentAndLinkBuffer_Struct> InOutFragmentAndLink;
//Buffer<uint> InFragmentListHead;
ByteAddressBuffer InFragmentListHead;

//////////////////////////////////////////////////////////////////////////
// HELPERS
//////////////////////////////////////////////////////////////////////////

int GetAddress(int2 nScreenPos, int nSample)
{
    uint offset = nSample * (uint)ScreenWidth * (uint)ScreenWidth;
    return (int)(4 * (((ScreenWidth * nScreenPos.y) + nScreenPos.x) + offset));
}

//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////

const static float CubeScale = 0.1f;

GSOutput FillOutput(float3 centerPos, float3 offset, float4 Colour)
{
	GSOutput output = (GSOutput)0;
	output.Pos = mul(float4(centerPos + offset * CubeScale, 1.0f), WorldViewProj);
	output.Colour = Colour;
	return output;
}

//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////

static const float fMult = 0.999f;

//////////////////////////////////////////////////////////////////////////

// point -> cube
[maxvertexcount(24)]
void main(point GSInput gin[1], 
            inout TriangleStream<GSOutput> triStream)
{
    int2 ScreenPos = int2(gin[0].Pos.xy);
    
    int Address = GetAddress(ScreenPos, SampleIndex);
    
    int nSample = (int)gin[0].Pos.z;
    
    uint CurrentIndex = InFragmentListHead.Load(Address);
    
    while (nSample-- > 0)
    {
        if (CurrentIndex != -1)
        {
            const FragmentAndLinkBuffer_Struct node = InOutFragmentAndLink[CurrentIndex];
            CurrentIndex = node.Next;
        }
    }
    
    if (CurrentIndex == -1)
    {
        return;
    }
    
    const FragmentData_Struct node = InOutFragmentAndLink[CurrentIndex].FragmentData;
    
    //float4 centerPos = mul(float4(ScreenPos.x, ScreenPos.y, node.Depth, 1.0f), RayBundleXFormInv);
    //centerPos /= centerPos.w;
    //float3 centerPos = float3(RayBundleOrigin.xy, RayBundleOrigin.z + node.Depth);
    
	//float4 centerPos = gin[0].Pos;
    float4 vOffset = mul(float4(ScreenPos.x * CubeScale, ScreenPos.y * CubeScale, 0.0f, 1.0f), RayBundleXFormInv);
    float4 centerPos = float4(vOffset.xyz + /*RayBundleOrigin.xyz +*/ RayBundleDirection.xyz * node.Depth * CubeScale, 1.0f);

	GSOutput output = (GSOutput)0;
    float4 Colour = node.Albedo;
	
	// All CCW RH coordinate system
	// -X
	{
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f, 0.5f * fMult, 0.5f * fMult), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f, 0.5f * fMult, -0.5f * fMult), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f, -0.5f * fMult, 0.5f * fMult), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f, -0.5f * fMult, -0.5f * fMult), Colour));
	}
	triStream.RestartStrip();
	// +X
	{
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f, 0.5f * fMult, -0.5f * fMult), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f, 0.5f * fMult, 0.5f * fMult), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f, -0.5f * fMult, -0.5f * fMult), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f, -0.5f * fMult, 0.5f * fMult), Colour));
	}
	triStream.RestartStrip();
	// -Y
	{
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, -0.5f, -0.5f * fMult), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, -0.5f, -0.5f * fMult), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, -0.5f, 0.5f * fMult), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, -0.5f, 0.5f * fMult), Colour));
	}
	triStream.RestartStrip();
	// +Y
	{
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, 0.5f, -0.5f * fMult), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, 0.5f, -0.5f * fMult), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, 0.5f, 0.5f * fMult), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, 0.5f, 0.5f * fMult), Colour));
	}
	triStream.RestartStrip();
	// -Z
	{
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, -0.5f * fMult, -0.5f), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, 0.5f * fMult, -0.5f), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, -0.5f * fMult, -0.5f), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, 0.5f * fMult, -0.5f), Colour));
	}
	triStream.RestartStrip();
	// +Z
	{
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, -0.5f * fMult, 0.5f), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, 0.5f * fMult, 0.5f), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, -0.5f * fMult, 0.5f), Colour));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, 0.5f * fMult, 0.5f), Colour));
	}
	triStream.RestartStrip();
}
