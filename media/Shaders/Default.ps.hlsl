////////////////////////////////////////////////////////////////////////////////

struct PS_INPUT
{
    float4 WPos		: POSITION;
    float4 Pos		: SV_POSITION;
    float4 Nrm		: NORMAL;
    float4 WNrm		: TEXCOORD0;
    float4 Colour	: COLOR;
    float2 UV		: TEXCOORD1;
    float4 Tangent	: TEXCOORD2;
    float4 Binormal	: TEXCOORD3;
};

////////////////////////////////////////////////////////////////////////////////

SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

////////////////////////////////////////////////////////////////////////////////

#include "common/common_defs.h"
#include "common/common.h"
#include "common/common_shadows.h"

////////////////////////////////////////////////////////////////////////////////

Texture2D DiffuseTexture;
Texture2D BumpTexture;

////////////////////////////////////////////////////////////////////////////////
// VOXEL CONE TRACING
////////////////////////////////////////////////////////////////////////////////

#if (VOXEL_CONE_TRACING == 1)

cbuffer VoxelConeTracing
{
	float VoxelSize;
	float VoxelSizeInv;
	float4x4 WorldToVoxel;
};

Texture3D VoxelOctreePosX;
Texture3D VoxelOctreeNegX;
Texture3D VoxelOctreePosY;
Texture3D VoxelOctreeNegY;
Texture3D VoxelOctreePosZ;
Texture3D VoxelOctreeNegZ;

float4 FetchVoxel(float3 VoxelOrigin, float3 SampleDir, float LOD)
{
	SampleDir.x *= -1.0f;
	float4 SampleX =
		SampleDir.x > 0.0
		? VoxelOctreePosX.SampleLevel(g_samPoint, VoxelOrigin, LOD)
		: VoxelOctreeNegX.SampleLevel(g_samPoint, VoxelOrigin, LOD);
	
	float4 SampleY = 
		SampleDir.y < 0.0
		? VoxelOctreePosY.SampleLevel(g_samPoint, VoxelOrigin, LOD)
		: VoxelOctreeNegY.SampleLevel(g_samPoint, VoxelOrigin, LOD);
	
	float4 SampleZ = 
		SampleDir.z < 0.0
		? VoxelOctreePosZ.SampleLevel(g_samPoint, VoxelOrigin, LOD)
		: VoxelOctreeNegZ.SampleLevel(g_samPoint, VoxelOrigin, LOD);
	
	float3 SampleWeights = abs(SampleDir);

	if (SampleX.w == 0.0)
	{
		SampleWeights.x = 0.0;
	}
	if (SampleY.w == 0.0)
	{
		SampleWeights.y = 0.0;
	}
	if (SampleZ.w == 0.0)
	{
		SampleWeights.z = 0.0;
	}
	
	float4 filtered = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	float fWeights = SampleWeights.x + SampleWeights.y + SampleWeights.z;
	if (fWeights > 1.0e-5f)
	{
		float InvSampleMag = 1.0f / fWeights;
		SampleWeights *= InvSampleMag;
		
		filtered = 
			SampleX * SampleWeights.x
			+ SampleY * SampleWeights.y
			+ SampleZ * SampleWeights.z;
	}
	
	return filtered;
}

float4 CalculateGlobalIllumination(float3 SampleOrigin, float3 ConeDirection,
	float ConeRatio, float TraceMaxDistance)
{
	float3 SamplePos = SampleOrigin;
	float4 GlobalIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// the starting sample diameter
	float MinDiameter = VoxelSize;

	// push out the starting point to avoid self-intersection
	float StartDist = MinDiameter;
	
	float Dist = StartDist;
	
	while (Dist <= TraceMaxDistance && GlobalIllumination.w < 1.0f)
	{
		// ensure the sample diameter is no smaller than the min
		// desired diameter for this cone (ensuring we always
		// step at least minDiameter each iteration, even for tiny
		// cones - otherwise lots of overlapped samples)
		float SampleDiameter = max(MinDiameter, ConeRatio * Dist);
		
		// convert diameter to LOD
		// for example:
		// log2(1/256 * 256) = 0
		// log2(1/128 * 256) = 1
		// log2(1/64 * 256) = 2
		float SampleLOD = log2(SampleDiameter * VoxelSizeInv);
		
		float3 SamplePos = SampleOrigin + ConeDirection * Dist;
		
		float4 SampleValue = FetchVoxel(SamplePos, -ConeDirection, SampleLOD);
		
		float SampleWeight = (1.0f - GlobalIllumination.w);
		GlobalIllumination += SampleValue * SampleWeight;
		
		Dist += SampleDiameter;
	}
	
	// decompress color range to decode limited HDR
	//accum.xyz *= 1.5;
	GlobalIllumination.xyz *= float3(0.5f, 0.5, 0.5f);
	
	return GlobalIllumination;
}

#elif (BIDIRECTIONAL_PATH_TRACING == 1)

//////////////////////////////////////////////////////////////////////////
// BIDIRECTION PATH TRACING
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Buffers

cbuffer RarelyModified
{
    float ScreenWidth : SCREENWIDTH;
    float ScreenHeight : SCREENHEIGHT;
    uint NumSamples : NUMSAMPLES;
};

//////////////////////////////////////////////////////////////////////////
// Structs

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

//////////////////////////////////////////////////////////////////////////

struct OneBounceIlluminationSampleData
{
	float3 Pos;
	float3 Normal;
	float4 DiffuseAndSpecular;
	float Roughness;
	float Intensity;
};

//////////////////////////////////////////////////////////////////////////

struct FragmentAndLinkBuffer_Struct
{
    FragmentData_Struct FragmentData;
    uint Next;
};

//////////////////////////////////////////////////////////////////////////

struct VPLSampleData_Struct
{
    float3 Position;
    float3 Normal;
    float4 Diffuse;
    float Roughness;
    float Intensity;
};

//////////////////////////////////////////////////////////////////////////

struct RayBundleCameraLocations
{
	float4x4 Transform;
	float4 Position;
	float4 Direction;
};

//////////////////////////////////////////////////////////////////////////
// Global buffers

StructuredBuffer<RayBundleCameraLocations> InRayBundleCameraLocations;
StructuredBuffer<VPLSampleData_Struct> InVPLSampleData;
StructuredBuffer<FragmentAndLinkBuffer_Struct> InOutFragmentAndLink;
ByteAddressBuffer InFragmentListHead;
//Buffer<uint> InFragmentListHead;
Texture2DArray InVPLShadowMaps;

//////////////////////////////////////////////////////////////////////////
// Helpers

int GetAddress(int2 nScreenPos, int nSample)
{
    uint SampleOffset = 4 * nSample * (uint)ScreenWidth * (uint)ScreenHeight;
    uint RowOffset = 4 * (uint)ScreenWidth * (uint)nScreenPos.y;
    uint ColumnOffset = 4 * (uint)nScreenPos.x;
    return ColumnOffset + RowOffset + SampleOffset;
}

//////////////////////////////////////////////////////////////////////////

float GetVPLPDF(float3 Dir)
{
    return 1.0f / (4.0f * PI);
}

//////////////////////////////////////////////////////////////////////////

float GetRayBundlePDF(float3 Dir)
{
    return 1.0f / (4.0f * PI);
}

//////////////////////////////////////////////////////////////////////////

float GetPDFSampledPathToReflectedPos(float3 ReflectPos, float3 ReflectPosNormal,
    float3 FragmentPos, float3 LightPos)
{
    float3 LightToReflectedPos = normalize(ReflectPos - LightPos);
    float3 ReflectPosToFragment = normalize(FragmentPos - ReflectPos);
    
    float ProbabilityLightToReflectedPos = GetVPLPDF(LightToReflectedPos);

    float3 FragmentToReflectPos = ReflectPos - FragmentPos;
    
    return (dot(FragmentToReflectPos, FragmentToReflectPos) / (dot(ReflectPosNormal, ReflectPosToFragment))) * ProbabilityLightToReflectedPos;
}

//////////////////////////////////////////////////////////////////////////

float GetPDFSampledPathToVPL(float3 VPLPos, float3 VPLNormal, float3 LightPos,
    float3 FragmentPos)
{
    float3 FragmentToVPL = normalize(VPLPos - FragmentPos);
    float3 VPLToLight = normalize(LightPos - VPLPos);
    
    float ProbabilityFragmentToVPL = GetRayBundlePDF(FragmentToVPL);
    
    float3 LightToVPL = VPLPos - LightPos;
    
    return (dot(LightToVPL, LightToVPL) / (dot(VPLNormal, VPLToLight))) * ProbabilityFragmentToVPL;
}

//////////////////////////////////////////////////////////////////////////

float GetPDFTwoBounce(float3 VPLPos, float3 VPLNormal,
	float3 ReflectedPos, float3 ReflectedNormal,
	float3 LightPos, float3 FragmentPos, float3 FragmentNormal,
	float3 CameraPos)
{
	float VPLToReflectedPosSq = max(dot(VPLPos - ReflectedPos, VPLPos - ReflectedPos), 1.0f);
	
	float VPLPDF = GetVPLPDF(normalize(VPLPos - LightPos));
	float CameraPDF = GetRayBundlePDF(normalize(FragmentPos - CameraPos));
	
	float3 VPLToLight = normalize(LightPos - VPLPos);
	float3 ReflectedPosToFragment = normalize(FragmentPos - ReflectedPos);
	
	float Denominator = dot(VPLNormal, VPLToLight) * dot(ReflectedNormal, ReflectedPosToFragment);
	
	return (VPLToReflectedPosSq * VPLPDF * CameraPDF) / Denominator;
}

//////////////////////////////////////////////////////////////////////////

    // NOTE: Currently we sample in a sphere around the scene for global ray
    // bundles. We only need to sample a hemisphere.
    // N.B This will also changed the PDF of the function to 1/2pi instead of 
    // 1/4pi

    // I = intensity
    // x0 = light pos
    // x1 = Reflected surface position
    // x2 = VPL
    // x3 = Fragment
    // x4 = camera
    // w01 = Light -> Reflected Surface Position
    // w02 = Light -> VPL
    // w31 = Fragment To ReflectedSurfacePos
    // w32 = Fragment To VPL
    // w34 = From fragment -> Eye (PosToEye)
    
    // y1 = weight 1 =  pow(pdf(x1), beta) / (pow(pdf(x1), beta) + pow(pdf(x2), beta));
    // Esentially this is the ratio of pdf x1 is to x1 + x2. This is always going
    // to be 50% for both for us since pdf(x1) == pdf(x2) since x1 and x2 are
    // illuminated from point light sources which distribute light evenly. THis
    // can be simplified to 0.5f for both y1 and y2.
    
    // y2 = weight 2 = read above.
    
    // f013 = BRDF(ReflectedSurfacePos, 
    //  ReflectedSurfacePos To Light, 
    //  ReflectedSurfacePos To Fragment)
    
    // f134 = BRDF(FragmentPos,
    //    Fragment To ReflectedSurfacePos,
    //    Fragment To Camera)
    
    // f023 = BRDF(VPLPos,
    //  VPLPos To Light,
    //  VPLPos To Fragment)
    
    // f234 = BRDF(Fragment,
    //  Fragment To VPL,
    //  Fragment To Camera)
    
    // n3 = FragmentSurfaceNormal
    // p1(x1) = PDF of sample path to reflected pos -> GetPDFSampledPathToReflectedPos
    // p2(x2) = PDF os sampled path to VPL -> GetPDFSampledPathToVPL
    
    // FirstPart = ((I(w01) * f013 * f134 * dot(n3, w31)) / p1(x1)) * y1
    // SecondPart = ((I(w02) * f023 * f234 * dot(n3, w32)) / p2(x2)) * y2 
    // Radiance = FirstPart + SecondPart;
    
//////////////////////////////////////////////////////////////////////////

float3 GetOneBounceIllumination_VPL(float3 LightPos, float3 CameraPos,
	OneBounceIlluminationSampleData Fragment,
	OneBounceIlluminationSampleData VPL)
{
	float3 FragmentToCamera = normalize(CameraPos - Fragment.Pos);

	// Clauclate f023 (BRDF between VPL, Light and Fragment)
	float3 VPLToLight = normalize(LightPos - VPL.Pos);
	float3 VPLToFragment = normalize(Fragment.Pos - VPL.Pos);
	float3 f023 = CookTorranceMicroFacetBRDF(
		VPLToLight, VPLToFragment, 
		VPL.Normal, VPL.Roughness, 
		float3(VPL.DiffuseAndSpecular.a, VPL.DiffuseAndSpecular.a, VPL.DiffuseAndSpecular.a),
		VPL.DiffuseAndSpecular.rgb);

	// Calculate f234 (BRDF between VPL, Fragment and Camera)
	float3 FragmentToVPL = normalize(VPL.Pos - Fragment.Pos);
	float3 f234 = CookTorranceMicroFacetBRDF(
		FragmentToVPL, FragmentToCamera, 
		Fragment.Normal, Fragment.Roughness, 
		float3(Fragment.DiffuseAndSpecular.a, Fragment.DiffuseAndSpecular.a, Fragment.DiffuseAndSpecular.a),
		Fragment.DiffuseAndSpecular.rgb);

	// Calculate radiance given from the VPL
	float3 RadianceFromVPL = VPL.Intensity * f023 * f234 * dot(Fragment.Normal, FragmentToVPL);

	// Calculate the PDF of the sampled direction reflections for both VPL and reflected pos
    float ProbabilityFromVPL = GetPDFSampledPathToVPL(VPL.Pos, VPL.Normal, LightPos,
		Fragment.Pos);
    
	// Calculate the weights of the radiance reflectance
    float VPLWeight = 0.5f;
    
	// Return the final radiance from one bounce illumination
    float3 Radiance = (RadianceFromVPL / ProbabilityFromVPL * VPLWeight);
        
    return Radiance; 
}

//////////////////////////////////////////////////////////////////////////

float3 GetOneBounceIllumination_GlobalRayBundle(float3 LightPos, float3 CameraPos,
	OneBounceIlluminationSampleData Fragment,
	OneBounceIlluminationSampleData Reflected)
{
	// Calculate f013 (BRDF of reflected pos, light and fragment)
	float3 ReflectedSurfacePosToLight = normalize(LightPos - Reflected.Pos);
	float3 ReflectedSurfaceToFragment = normalize(Fragment.Pos - Reflected.Pos);
	float3 f013 = CookTorranceMicroFacetBRDF(
		ReflectedSurfacePosToLight, ReflectedSurfaceToFragment, 
		Reflected.Normal, Reflected.Roughness, 
		float3(Reflected.DiffuseAndSpecular.a, Reflected.DiffuseAndSpecular.a, Reflected.DiffuseAndSpecular.a),
		Reflected.DiffuseAndSpecular.rgb);
	return Reflected.DiffuseAndSpecular.rgb;

	// Calculate f134 (BRDF between fragment, camera and reflected pos)
	float3 FragmentToReflectedSurfacePos = normalize(Reflected.Pos - Fragment.Pos);
	float3 FragmentToCamera = normalize(CameraPos - Fragment.Pos);
	float3 f134 = CookTorranceMicroFacetBRDF(
		FragmentToReflectedSurfacePos, FragmentToCamera, 
		Fragment.Normal, Fragment.Roughness, 
		float3(Fragment.DiffuseAndSpecular.a, Fragment.DiffuseAndSpecular.a, Fragment.DiffuseAndSpecular.a),
		Fragment.DiffuseAndSpecular.rgb);

	// Calculate radiance given from the reflected pos
	float3 RadianceFromReflectedPos = Reflected.Intensity * f013 * f134 * dot(Fragment.Normal, FragmentToReflectedSurfacePos);

	return RadianceFromReflectedPos;

	// Calculate the PDF of the sampled direction reflections for both VPL and reflected pos
    float ProbabilityFromReflectPos = GetPDFSampledPathToReflectedPos(
		Reflected.Pos, Reflected.Normal, Fragment.Pos, LightPos);
    
	// Calculate the weights of the radiance reflectance
    float ReflectPosWeight = 0.5f;
    
	// Return the final radiance from one bounce illumination
    float3 Radiance = (RadianceFromReflectedPos / ProbabilityFromReflectPos * ReflectPosWeight);
        
    return Radiance; 
}

//////////////////////////////////////////////////////////////////////////

float3 GetTwoBounceIllumination(float3 LightPos, float3 CameraPos,
	OneBounceIlluminationSampleData Fragment,
	OneBounceIlluminationSampleData Reflected,
	OneBounceIlluminationSampleData VPL)
{
	// VPL BRDF (Between light and reflected pos)
	float3 VPLToLight = normalize(LightPos - VPL.Pos);
	float3 VPLToReflectedPos = normalize(Reflected.Pos - VPL.Pos); 
	float3 f012 = CookTorranceMicroFacetBRDF(
		VPLToLight, VPLToReflectedPos, 
		VPL.Normal, VPL.Roughness, 
		float3(VPL.DiffuseAndSpecular.a, VPL.DiffuseAndSpecular.a, VPL.DiffuseAndSpecular.a),
		VPL.DiffuseAndSpecular.rgb);
	
	// Reflected Position BRDF (between VPL and Fragment)
	float3 ReflectedPosToVPL = normalize(VPL.Pos - Reflected.Pos);
	float3 ReflectedPosToFragment = normalize(Fragment.Pos - Reflected.Pos);
	float3 f123 = CookTorranceMicroFacetBRDF(
		ReflectedPosToVPL, ReflectedPosToFragment, 
		Reflected.Normal, Reflected.Roughness, 
		float3(Reflected.DiffuseAndSpecular.a, Reflected.DiffuseAndSpecular.a, Reflected.DiffuseAndSpecular.a),
		Reflected.DiffuseAndSpecular.rgb);
		
	// Fragment BRDF (Between Reflected pos and Camera)
	float3 FragmentToReflectedPos = normalize(Reflected.Pos - Fragment.Pos);
	float3 FragmentToCamera = normalize(CameraPos - Fragment.Pos);
	
	float3 f234 = CookTorranceMicroFacetBRDF(
		FragmentToReflectedPos, FragmentToCamera, 
		Fragment.Normal, Fragment.Roughness, 
		float3(Fragment.DiffuseAndSpecular.a, Fragment.DiffuseAndSpecular.a, Fragment.DiffuseAndSpecular.a),
		Fragment.DiffuseAndSpecular.rgb);
	
	float DotFragmentToReflected = dot(Fragment.Normal, FragmentToReflectedPos);
	
	float3 Numerator = VPL.Intensity * f012 * f123 * f234 * DotFragmentToReflected;
	
	float PDF = GetPDFTwoBounce(
		VPL.Pos, VPL.Normal,
		Reflected.Pos, Reflected.Normal,
		LightPos, Fragment.Pos, Fragment.Normal, CameraPos);
	
	return Numerator / PDF;
	
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
	if (dir >= 0.0f)
	{
		while (index != -1)
		{
			const FragmentAndLinkBuffer_Struct node = InOutFragmentAndLink[index];
			
			if (node.FragmentData.Depth > 0.0f &&
				node.FragmentData.Depth < data.Depth &&
				node.FragmentData.Depth > KeyDepth)
			{
				data = node.FragmentData;
				break;
			}
			
			index = node.Next;
		}
	}
	else
	{
		while (index != -1)
		{
			const FragmentAndLinkBuffer_Struct node = InOutFragmentAndLink[index];
			
			if (node.FragmentData.Depth < 0.0f &&
				node.FragmentData.Depth > data.Depth &&
				node.FragmentData.Depth < KeyDepth)
			{
				data = node.FragmentData;
				break;
			}
			
			index = node.Next;
		}
	}
    
    data.Depth = abs(data.Depth);
    
    return data;
}

//////////////////////////////////////////////////////////////////////////

float3 Integrate(float3 LightPos, float3 CameraPos,
	OneBounceIlluminationSampleData tFragmentData, uint uIdx)
{
    float3 IndirectIllumination = float3(0.0f, 0.0f, 0.0f);

    VPLSampleData_Struct tVPL = InVPLSampleData.Load(uIdx);

	float3 WorldPos = tVPL.Position;
	float3 WorldNormal = tVPL.Normal;
		
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

	RayBundleCameraLocations tLocation = InRayBundleCameraLocations.Load(uIdx);
	float4 fXY = mul(float4(tFragmentData.Pos, 1.0f), tLocation.Transform);
	fXY = float4((fXY.x + 1.0f) * 0.5f, ((fXY.y + 1.0f) * 0.5f), 0.0f, 0.0f); 
    fXY.xy *= float2(ScreenWidth, ScreenHeight);
    
    fXY.xy = trunc(fXY.xy + float2(0.5f, 0.5f));
	int2 nXY = int2(fXY.xy);
	int Address = GetAddress(nXY, (int)uIdx);

	float Depth = length(tFragmentData.Pos - tLocation.Position.xyz);
    float Dir = sign(dot(tFragmentData.Normal, tLocation.Direction.xyz));

	uint index = InFragmentListHead.Load(Address);                                             
    
    if (index != -1)
    {
        uint status;
		FragmentAndLinkBuffer_Struct node = InOutFragmentAndLink.Load(index, status);

		//float Dir = node.FragmentData.Depth > 0.0f ? -1.0f : 1.0f;
		FragmentData_Struct tStruct = FindFragment(node.Next, Dir, node.FragmentData.Depth);

		if (abs(tStruct.Depth) < FLT_MAX)
		{
			OneBounceIlluminationSampleData tReflectedData = (OneBounceIlluminationSampleData)0;

			// TODO: Find the actual reflected position
			float fDistance = (tStruct.Depth - Depth);
			tReflectedData.Pos = tFragmentData.Pos + tLocation.Direction.xyz * fDistance;
			tReflectedData.Normal = tStruct.Normal.xyz;
			tReflectedData.DiffuseAndSpecular = tStruct.Albedo;
			tReflectedData.Roughness = tStruct.Roughness;
			// TODO: Find the actual intensity
			float3 ToReflectedPos = (LightPos - tReflectedData.Pos);
			tReflectedData.Intensity = Intensity / (dot(ToReflectedPos, ToReflectedPos) + 1.0f);
			IndirectIllumination += saturate(GetOneBounceIllumination_GlobalRayBundle(
				LightPos, CameraPos, 
				tFragmentData, 
				tReflectedData));
		}
    }

	float4x4 VPLView = CreateLookAtLH(float3(0.0f, 0.0f, 0.0f), WorldNormal, 
    	UpDirection);
		
	float4x4 VPLXForm = float4x4(
		float4(1.0f, 0.0f, 0.0f, 0.0f),
		float4(0.0f, 1.0f, 0.0f, 0.0f),
		float4(0.0f, 0.0f, 1.0f, 0.0f),
		float4(-WorldPos, 1.0f));
	float4x4 WorldViewProjLight = mul(VPLXForm, VPLView);

    bool bInShadow = IsInDualParabaloidShadowMap(
		InVPLShadowMaps, (float)uIdx,
		WorldViewProjLight, tFragmentData.Pos, false);

	OneBounceIlluminationSampleData tVPLIlluminationData = (OneBounceIlluminationSampleData)tVPL;

	if (!bInShadow)
	{
        float3 Illumination = saturate(GetOneBounceIllumination_VPL(
			LightPos, CameraPos, 
			tFragmentData, 
			tVPLIlluminationData));
		IndirectIllumination += Illumination;
	}
    
    return IndirectIllumination;
}

//////////////////////////////////////////////////////////////////////////

float3 IntegrateGlobalIllumination(float3 LightPos, float3 CameraPos,
	OneBounceIlluminationSampleData tFragmentData)
{
	float3 IndirectIllumination = float3(0.0f, 0.0f, 0.0f);

	for (uint uIdx=0; uIdx < NumSamples; ++uIdx) {
		IndirectIllumination += Integrate(LightPos, CameraPos, tFragmentData, uIdx);
	}
	IndirectIllumination /= NumSamples;

	return IndirectIllumination;
}

//////////////////////////////////////////////////////////////////////////

#endif // (BIDIRECTION_PATH_TRACING == 1) || (VOXEL_CONE_TRACING == 1)

static const float BumpConstant = 1.0f;

float4 main(PS_INPUT Input) : SV_TARGET0
{
	// Calculate the normal, including the information in the bump map
	float4 FragmentNormal = normalize(Input.Nrm);
	float4 BumpSample = BumpConstant * (BumpTexture.Sample(g_samPoint, Input.UV) * 2.0f - 1.0f);
	float3 NormalWithBump = normalize(((BumpSample.x * Input.Tangent) + (BumpSample.y * Input.Binormal) + (BumpSample.z * FragmentNormal)).xyz);

	float3 ToCamera = normalize(mul(View.xyz - Input.WPos.xyz, (float3x3)WorldInvTranspose));
	float3 ToLight = normalize(mul(LightPos.xyz - Input.WPos.xyz, (float3x3)WorldInvTranspose));

	float DistSqToLight = dot(LightPos.xyz - Input.WPos.xyz,
		LightPos.xyz - Input.WPos.xyz);
	const float Radius = LightRadius;

	float LightRadius = Square(saturate(1.0f - Square(DistSqToLight * Square(1.0f / Radius)))) / (DistSqToLight + 1.0f);
	LightRadius = saturate(LightRadius * Intensity);
	
	float3 SpecularColour = float3(LightSpecular, LightSpecular, LightSpecular);

	float LightModifier = GetShadowModifier(Input.WPos.xyz) * LightRadius;

	float3 DiffuseSample = DiffuseTexture.Sample(g_samPoint, Input.UV).xyz;
	float3 TextureColour = CookTorranceMicroFacetBRDF(ToLight, ToCamera, 
		NormalWithBump, Roughness, SpecularColour, DiffuseSample);
	  
	float4 OutputColour = float4(0.0f, 0.0f, 0.0f, 1.0f);
	  
#if !defined(NO_DIRECT_LIGHT)
	OutputColour += float4(TextureColour * LightModifier, 1.0f);
#endif

	// indirect diffuse
#if (VOXEL_CONE_TRACING == 1)
	// Position of fragment in voxel texture space
	float4 VoxelWorldPos = mul(float4(Input.WPos.xyz, 1.0f), WorldToVoxel);
	VoxelWorldPos.xyz *= 1.0f / VoxelWorldPos.w;

	float ConeRatio = 1.0f;
	float MaxDist = 0.5f;
		
	OutputColour.xyz += CalculateGlobalIllumination(VoxelWorldPos.xyz, 
		NormalWithBump, ConeRatio, MaxDist).xyz;

	OutputColour.xyz += 0.707f * ConeRatio * CalculateGlobalIllumination(
		VoxelWorldPos.xyz, normalize(NormalWithBump + Input.Tangent.xyz), ConeRatio, MaxDist).xyz;
	OutputColour.xyz += 0.707f * ConeRatio * CalculateGlobalIllumination(
		VoxelWorldPos.xyz, normalize(NormalWithBump - Input.Tangent.xyz), ConeRatio, MaxDist).xyz;
	OutputColour.xyz += 0.707f * ConeRatio * CalculateGlobalIllumination(
		VoxelWorldPos.xyz, normalize(NormalWithBump + Input.Binormal.xyz), ConeRatio, MaxDist).xyz;
	OutputColour.xyz += 0.707f * ConeRatio * CalculateGlobalIllumination(
		VoxelWorldPos.xyz, normalize(NormalWithBump - Input.Binormal.xyz), ConeRatio, MaxDist).xyz;
	
#elif (BIDIRECTIONAL_PATH_TRACING == 1)
	OneBounceIlluminationSampleData FragmentData;
	FragmentData.Pos = Input.WPos.xyz;
	FragmentData.Normal = NormalWithBump;
	FragmentData.DiffuseAndSpecular = float4(DiffuseSample, SpecularColour.r);
	FragmentData.Intensity = LightRadius;
	FragmentData.Roughness = 0.5f;
	OutputColour.xyz += IntegrateGlobalIllumination(LightPos.xyz, View.xyz,
		FragmentData);
#endif

	return saturate(OutputColour);	
}