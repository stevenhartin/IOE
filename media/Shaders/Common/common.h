#ifndef _IOE_SHADER_COMMON_H_
#define _IOE_SHADER_COMMON_H_

////////////////////////////////////////////////////////////////////////////////

static const uint NUMBER_OF_SAMPLES = 4;

////////////////////////////////////////////////////////////////////////////////

cbuffer GlobalSettings
{
    float4 View : VIEW;
    float4x4 WorldViewProj : WORLDVIEWPROJ;
    float4x4 WorldInvTranspose : WORLDINVTRANSPOSE;
	float2 ViewportSize : VIEWPORTSIZE;
};

////////////////////////////////////////////////////////////////////////////////

cbuffer LightSettings
{
    float Intensity : INTENSITY;
    float Roughness : ROUGHNESS;
    float Near : NEAR;
    float Far : FAR;
    float4 LightPos  : LIGHTPOS;
    float4 LightColour  : LIGHTCOLOUR;
	float LightSpecular : LIGHTSPECULAR;
	float LightRadius : LIGHTRADIUS;
};

////////////////////////////////////////////////////////////////////////////////

float Square(float x)
{
    return x * x;
}

////////////////////////////////////////////////////////////////////////////////

float RadicalInverseVDC(uint bits) 
{
    volatile uint bitscopy = bits;
    bitscopy = ((bitscopy << 16u) | (bitscopy >> 16u));
    bitscopy = ((bitscopy & 0x55555555u) << 1u) | ((bitscopy & 0xAAAAAAAAu) >> 1u);
    bitscopy = ((bitscopy & 0x33333333u) << 2u) | ((bitscopy & 0xCCCCCCCCu) >> 2u);
    bitscopy = ((bitscopy & 0x0F0F0F0Fu) << 4u) | ((bitscopy & 0xF0F0F0F0u) >> 4u);
    bitscopy = ((bitscopy & 0x00FF00FFu) << 8u) | ((bitscopy & 0xFF00FF00u) >> 8u);
    return float(bitscopy) * 2.3283064365386963e-10; // / 0x100000000
}

////////////////////////////////////////////////////////////////////////////////

//The ith point xi is then computed by         
float2 Hammersley(uint i, uint N)
{
    return float2(float(i)/float(N), RadicalInverseVDC(i));
}

////////////////////////////////////////////////////////////////////////////////

float3 ImportanceSampleGGX( float2 Xi, float Roughness, float3 N )
{
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );
    float3 H;
    H.x = SinTheta * cos( Phi );
    H.y = SinTheta * sin( Phi );
    H.z = CosTheta;
    float3 UpVector = abs(N.z) < 0.999 ? float3(0,0,1) : float3(1,0,0);
    float3 TangentX = normalize( cross( UpVector, N ) );
    float3 TangentY = cross( N, TangentX );
    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

////////////////////////////////////////////////////////////////////////////////

float GGX(float NdotV, float a)
{
	float k = a / 2;
	return NdotV / (NdotV * (1.0f - k) + k);
}

////////////////////////////////////////////////////////////////////////////////

float G_Smith(float a, float nDotV, float nDotL)
{
	return GGX(nDotL, a * a) * GGX(nDotV, a * a);
}

////////////////////////////////////////////////////////////////////////////////

float3 SpecularIBL( float3 SpecularColor , float Roughness, float3 N, float3 V )
{
    float3 SpecularLighting = 0;
    for( uint i = 0; i < NUMBER_OF_SAMPLES; i++ )
    {
        float2 Xi = Hammersley( i, NUMBER_OF_SAMPLES );

        float3 H = ImportanceSampleGGX( Xi, Roughness, N );
        float3 L = 2 * dot( V, H ) * H - V;
        float NoV = saturate( dot( N, V ) );
        float NoL = saturate( dot( N, L ) );
        float NoH = saturate( dot( N, H ) );
        float VoH = saturate( dot( V, H ) );
        if( NoL > 0 )
        {
            float3 SampleColor = /*EnvMap.SampleLevel( EnvMapSampler , L, 0 ).rgb*/float3(0.8f, 0.8f, 0.8f);
            float G = G_Smith( Roughness, NoV, NoL );
            float Fc = pow( 1 - VoH, 5 );
            float3 F = (1 - Fc) * SpecularColor + Fc;
            // Incident light = SampleColor * NoL
            // Microfacet specular = D*G*F / (4*NoL*NoV)
            // pdf = D * NoH / (4 * VoH)
            SpecularLighting += SampleColor * F * G * VoH / (NoH * NoV);
        }
    }
    return SpecularLighting / NUMBER_OF_SAMPLES;
}

////////////////////////////////////////////////////////////////////////////////

float2 IntegrateBRDF( float Roughness, float NoV )
{
	float3 V;
	V.x = sqrt( 1.0f - NoV * NoV ); // sin
	V.y = 0.0;
	V.z = NoV;
	// cos
	float A = 0.0;
	float B = 0.0;

	for( uint i = 0u; i < NUMBER_OF_SAMPLES; i++ )
	{
		float2 Xi = Hammersley( i, NUMBER_OF_SAMPLES );
		float3 H = ImportanceSampleGGX( Xi, Roughness, float3(0.0,0.0,1.0) );
		float3 L = 2.0 * dot( V, H ) * H - V;
		float NoL = max( L.z, 0.0 );
		float NoH = max( H.z, 0.0 );
		float VoH = max( dot( V, H ), 0.0 );

		if( NoL > 0 )
		{
			float G = G_Smith( Roughness, NoV, NoL );
			float G_Vis = G * VoH / (NoH * NoV);
			float Fc = pow( 1.0 - VoH, 5.0 );
			A += (1.0 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	return float2(  (A)/float(NUMBER_OF_SAMPLES), (B)/float(NUMBER_OF_SAMPLES)) ;
}

////////////////////////////////////////////////////////////////////////////////

float3 ApproximateSpecularIBL( float3 SpecularColor , float Roughness, float3 N, float3 V )
{
	float NoV = saturate( dot( N, V ) );
	float3 R = 2 * dot( V, N ) * N - V;
	float3 PrefilteredColor = /*PrefilterEnvMap( Roughness, R )*/float3(1.0f, 1.0f, 1.0f);
	float2 EnvBRDF = IntegrateBRDF( Roughness, NoV );
	return PrefilteredColor * ( SpecularColor * EnvBRDF.x + EnvBRDF.y );
}

////////////////////////////////////////////////////////////////////////////////

float D_GGX( float Roughness, float NoH )
{
    float m = Roughness * Roughness;
	float m2 = m * m;
	float d = ( NoH * m2 - NoH ) * NoH + 1;	// 2 mad
	return m2 / ( PI*d*d );
}

////////////////////////////////////////////////////////////////////////////////

float3 F_Schlick( float3 SpecularReflectance, float VoH )
{
	//float Fc = pow( 1 - VoH, 5 );							// 1 sub, 3 mul
	float SphericalGaussianApproximation = exp2((-5.55473f * VoH - 6.98316f) * VoH );	// 1 mad, 1 mul, 1 exp
	//return Fc + (1 - Fc) * SpecularColor;					// 1 add, 3 mad
	
	// Anything less than 2% is physically impossible and is instead considered to be shadowing
	//return saturate( 50.0 * SpecularColor.g ) * Fc + (1 - Fc) * SpecularColor;
    return SpecularReflectance + (1.0f - SpecularReflectance) * SphericalGaussianApproximation;	
}

////////////////////////////////////////////////////////////////////////////////

float Vis_SmithJointApprox( float Roughness, float NoV, float NoL )
{
	float a =  Roughness * Roughness;
    float k = ((Roughness + 1.0) * (Roughness + 1.0)) * 0.125f;

    float G1 = NoL / (NoL * (1.0 - k) + k);
    float G2 = NoV / (NoV * (1.0 - k) + k);

	//float Vis_SmithV = NoV * ( NoV * ( 1 - a ) + a );
	//float Vis_SmithL = NoL * ( NoL * ( 1 - a ) + a );

	//return 0.5 * rcp( Vis_SmithV + Vis_SmithL );
    return G1 * G2;
}

////////////////////////////////////////////////////////////////////////////////

float3 Diffuse_Lambert( float3 DiffuseColor )
{
	return DiffuseColor * (1.0 / PI);
}

////////////////////////////////////////////////////////////////////////////////

float3 CookTorranceMicroFacetBRDF(float3 ToLight, float3 ToCamera, 
    float3 SurfaceNormal, float Roughness, float3 SpecularReflectance,
    float3 DiffuseTexture)
{
    // SpecularReflectance should default to (0.04f, 0.04f, 0.04f) for non-metals

    // f(l, v) = D(h) * F(v, h) * G(l, v, h)
    //           ---------------------------
    //                   4 (n.l)(n.v)
    //
    // h -> normalize(v + l)
    //  
    // Specular D = NDF -> Disney's choice which is GGX / Trowbridge-Reitz
    //  D(h) = roughness^2 / (pi * ((n.h)^2(roughness^2-1)+1))^2)
    //
    // Geometrix Attenuation (G) -> Schlick
    //    k = ((Roughness + 1) ^ 2) / 8
    // G1(v) = n.v / ((n.v)(1 - k) + k)
    // G(l, v, h) = G1(l)G1(v)
    //
    // Fresnel -> Schlick's approximation
    // F(v, h) = F0 + (1 - F0) * 2^((-5.55473(v.h)-6398316)(v.h))
    // F0 -> Specular reflectance at normal incidence
    float3 h = normalize(ToCamera + ToLight);
    
    float NoH = saturate(dot(SurfaceNormal, h));
    float VoH = saturate(dot(ToCamera, h));
    float NoL = saturate(dot(SurfaceNormal, ToLight));
    float NoV = abs(dot(SurfaceNormal, ToCamera)) + 1e-5f;
    
    float D = D_GGX(Roughness, NoH);
    float3 F = F_Schlick(SpecularReflectance, VoH);
    float G = Vis_SmithJointApprox(Roughness, NoV, NoL);
    
    float3 Diffuse = Diffuse_Lambert(DiffuseTexture); 
    
    return Diffuse + ((D * F * G) / (4.0f * NoL * NoV));
}

////////////////////////////////////////////////////////////////////////////////

#endif // _IOE_SHADER_COMMON_H_