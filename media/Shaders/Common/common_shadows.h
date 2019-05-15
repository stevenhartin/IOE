#ifndef _IOE_SHADER_COMMON_SHADOWS_H_
#define _IOE_SHADER_COMMON_SHADOWS_H_

////////////////////////////////////////////////////////////////////////////////

SamplerState g_ShadowSampler
{
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = Border;
	AddressV = Border;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
};

////////////////////////////////////////////////////////////////////////////////

#if (SHADOW_MAPPING == 1)
// Cube mapping
TextureCube ShadowMap;

#elif (SHADOW_MAPPING == 2)
// Parabaloid shadow mapping, 2 element array (front / back)
Texture2DArray ShadowMap;

////////////////////////////////////////////////////////////////////////////////

cbuffer ParabaloidShadowMapping 
{
    float4x4 ParabaloidView;
}

////////////////////////////////////////////////////////////////////////////////

#endif

////////////////////////////////////////////////////////////////////////////////
///
//http://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
float DepthBufferValueToWorldDistance(float DepthBufferValue)
{
	float z_n = /*2.0f * DepthBufferValue - 1.0f*/DepthBufferValue;
	float z_e = 2.0f * Near * Far / (Far + Near - z_n * (Far - Near));
	return z_e;
}

float WorldDistanceToDepthBufferValue(float WorldDistance)
{
	float A = WorldViewProj[2][3];
	float B = WorldViewProj[3][3];
	return 0.5f * (-A * WorldDistance + B) / WorldDistance + 0.5f;
}

////////////////////////////////////////////////////////////////////////////////

bool IsInCubemapShadow(float ShadowDepth, float3 LightPosition, float3 VertexPosition)
{
    float3 LightDir = LightPosition.xyz - VertexPosition;
    float LightDistance = length(LightDir) - Tolerance;
    
    ShadowDepth = max(ShadowDepth, Tolerance);
    
    bool InShadow = false;

    if (LightDistance > ShadowDepth)
    {
        InShadow = true;
    }

    return InShadow;
}

////////////////////////////////////////////////////////////////////////////////

bool IsInDualParabaloidShadowMap(Texture2DArray ShadowMapBuffer, float Offset,
    float4x4 LightView, float3 VertexPosition, bool bCheckReverse=true,
	float fTolerance=Tolerance)
{
    // Parabaloid shadow mapping
	
    // texcoord-calculation is the same calculation as in the Depth-VS,
    // but texcoords have to be in range [0, 1]
    
    // transform into lightspace
	float3 ChangedVertexPosition = VertexPosition.xyz;
    float3 vPosDP = mul(float4(ChangedVertexPosition, 1.0f), LightView).xyz;	
    
    float fLength = length(vPosDP);

    vPosDP /= fLength;
    
    // compute and read according depth
    float fDPDepth=0.0f;
    float fSceneDepth=(fLength - Near) / (Far - Near);
    
    if(vPosDP.z >= 0.0f)
    {		
        float2 vTexFront;
        vTexFront.x =  (vPosDP.x /  (1.0f + vPosDP.z)) * 0.5f + 0.5f; 
        vTexFront.y =  1.0f - ((vPosDP.y /  (1.0f + vPosDP.z)) * 0.5f + 0.5f); 	
        
        float4 vSample = ShadowMapBuffer.Sample(g_ShadowSampler, 
            float3(vTexFront, Offset + 0.0f));
        if (vSample.a <= 0.0f)
        {
        	fDPDepth = -FLT_MAX;
		}
		else
		{
			fDPDepth = vSample.r;
		}
    }
    else if (bCheckReverse)
    {
        // for the back the z has to be inverted		
        float2 vTexBack;
        vTexBack.x =  (vPosDP.x /  (1.0f - vPosDP.z)) * 0.5f + 0.5f; 
        vTexBack.y =  1.0f - ((vPosDP.y /  (1.0f - vPosDP.z)) * 0.5f + 0.5f); 
        
        fDPDepth = ShadowMapBuffer.Sample(g_ShadowSampler, 
            float3(vTexBack, Offset + 1.0f)).r;
    }
        
    bool bInShadow = false;
    
    if((fDPDepth + fTolerance) < fSceneDepth)
    {
        bInShadow = true;
    }

    return bInShadow;
}

////////////////////////////////////////////////////////////////////////////////

#if (SHADOW_MAPPING == 1)

float GetShadowModifier_Version(float3 VertexPosition)
{
    // Cube shadow mapping
    float3 LightDir = normalize(LightPos.xyz - VertexPosition);
    
    float ShadowMapDepth = ShadowMap.Sample(g_ShadowSampler, 
        float3(-LightDir.x, -LightDir.y, -LightDir.z)).r;
    
    float fReturn = 1.0f;
    
    if (IsInCubemapShadow(ShadowMapDepth, LightPos.xyz, VertexPosition))
    {
        // In shadow
        fReturn = 0.0f;
    }

    // Not in shadow
    return fReturn;
}

#endif

////////////////////////////////////////////////////////////////////////////////

#if (SHADOW_MAPPING == 2)

float GetShadowModifier_Version(float3 VertexPosition)
{
    bool IsInShadowMap = IsInDualParabaloidShadowMap(ShadowMap, 0.0f,
        ParabaloidView, VertexPosition);
    
    float fReturn = 1.0f;
    
    if (IsInShadowMap)
    {
        fReturn = 0.0f;
    }

    return fReturn;
}

#endif

////////////////////////////////////////////////////////////////////////////////
                                                                                
float GetShadowModifier(float3 VertexPosition)
{
#if (defined(SHADOW_MAPPING))
    return GetShadowModifier_Version(VertexPosition);
#else
    return 1.0f;
#endif
}

////////////////////////////////////////////////////////////////////////////////

#endif // _IOE_SHADER_COMMON_SHADOWS_H_