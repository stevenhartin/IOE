cbuffer GlobalSettings
{
    float4 View : VIEW;
	float4x4 WorldViewProj : WORLDVIEWPROJ;
    float4x4 WorldInvTranspose : WORLDINVTRANSPOSE;
};

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

SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

Texture2D DiffuseTexture;
Texture2D BumpTexture;

static const float BumpConstant = 1.0f;
static const float PI = 3.14159265f;

/*float4 main(PS_INPUT input) : SV_TARGET0
{
    // Calculate the normal, including the information in the bump map
	float4 bumpSample = BumpConstant * (BumpTexture.Sample(g_samPoint, input.UV) * 2.0f - 1.0f);
    float3 bumpNormal = normalize(((bumpSample.x * input.Tangent) + (bumpSample.y * input.Binormal) + (bumpSample.z * input.Nrm)).xyz);

    // Calculate the diffuse light component with the bump map normal
	float3 light = normalize(float3(-0.5773502691896257f, -0.5773502691896257f, 0.5773502691896257f));

    float diffuseIntensity = dot(light, bumpNormal);
    if(diffuseIntensity < 0)
	{
        diffuseIntensity = 0;
	}

    // Calculate the specular light component with the bump map normal
    /*float3 r = normalize(2 * dot(light, bumpNormal) * bumpNormal - light);
    float3 v = normalize(mul(normalize(ViewVector), World));
    float dotProduct = dot(r, v);

    float4 specular = SpecularIntensity * SpecularColor * max(pow(dotProduct, Shininess), 0) * diffuseIntensity;* /

    // Calculate the texture color
    float4 textureColor = DiffuseTexture.Sample(g_samPoint, input.UV) * input.Colour;

    // Combine all of these values into one (including the ambient light)
    return saturate(textureColor * (diffuseIntensity)/* + AmbientColor * AmbientIntensity + specular* /);
}*/

/*
float4 CalculateLighting(float4 inColour, float4 inNrm,
    const float3 lightdir, const float4 lightcolour)
{
	float fDot = dot(inNrm.xyz, lightdir);
	if (fDot > 0.0f)
	{	
		return ((inColour * lightcolour) * fDot);
	}
	else
	{
		float4 returnColour = float4(0.0f, 0.0f, 0.0f, 1.0f);
		return returnColour;
	}
}

float4 main(PS_INPUT input) : SV_TARGET0
{
    float4 sample = DiffuseTexture.SampleLevel(g_samPoint, input.UV, 0) * input.Colour;
    float4 output = CalculateLighting(sample, input.Nrm, float3(-0.5773502691896257f, 0.5773502691896257f, 0.5773502691896257f), float4(1.0f, 1.0f, 1.0f, 1.0f)) + float4(0.25f, 0.25f, 0.25f, 0.0f) * input.Colour;
    output += CalculateLighting(sample, input.Nrm, float3(0.5773502691896257f, 0.5773502691896257f, -0.5773502691896257f), float4(0.66f, 0.66, 0.66, 0.66));
    
    return saturate(output);
};
*/

/*float radicalInverse_VdC(uint bits) 
{
	uint bitscopy = bits;
    bitscopy = (bitscopy << 16u) | (bitscopy >> 16u);
    bitscopy = ((bitscopy  0x55555555u) << 1u) | ((bitscopy  0xAAAAAAAAu) >> 1u);
    bitscopy = ((bitscopy  0x33333333u) << 2u) | ((bitscopy  0xCCCCCCCCu) >> 2u);
    bitscopy = ((bitscopy  0x0F0F0F0Fu) << 4u) | ((bitscopy  0xF0F0F0F0u) >> 4u);
    bitscopy = ((bitscopy  0x00FF00FFu) << 8u) | ((bitscopy  0xFF00FF00u) >> 8u);
    return float(bitscopy) * 2.3283064365386963e-10; // / 0x100000000
}*/

static const float3 g_LightPos = float3(2.5f, 10.0f, 1.0f);

//The ith point xi is then computed by         
float2 Hammersley(uint i, uint N)
{
    return float2(float(i)/float(N), /*radicalInverse_VdC(i)*/0);
}

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

float GGX(float NdotV, float a)
{
	float k = a / 2;
	return NdotV / (NdotV * (1.0f - k) + k);
}

float G_Smith(float a, float nDotV, float nDotL)
{
	return GGX(nDotL, a * a) * GGX(nDotV, a * a);
}


float3 SpecularIBL( float3 SpecularColor , float Roughness, float3 N, float3 V )
{
    float3 SpecularLighting = 0;
	const uint NumSamples = 1024;
    for( uint i = 0; i < NumSamples; i++ )
    {
        float2 Xi = Hammersley( i, NumSamples );

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
    return SpecularLighting / NumSamples;
}

float2 IntegrateBRDF( float Roughness, float NoV )
{
	float3 V;
	V.x = sqrt( 1.0f - NoV * NoV ); // sin
	V.y = 0.0;
	V.z = NoV;
	// cos
	float A = 0.0;
	float B = 0.0;

	const uint NUMBER_OF_SAMPLES = 1024;

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

float3 ApproximateSpecularIBL( float3 SpecularColor , float Roughness, float3 N, float3 V )
{
	float NoV = saturate( dot( N, V ) );
	float3 R = 2 * dot( V, N ) * N - V;
	float3 PrefilteredColor = /*PrefilterEnvMap( Roughness, R )*/float3(0.8f, 0.8f, 0.8f);
	float2 EnvBRDF = IntegrateBRDF( Roughness, NoV );
	return PrefilteredColor * ( SpecularColor * EnvBRDF.x + EnvBRDF.y );
}


float D_GGX( float Roughness, float NoH )
{
	float m = Roughness * Roughness;
	float m2 = m * m;
	float d = ( NoH * m2 - NoH ) * NoH + 1;	// 2 mad
	return m2 / ( PI*d*d );					// 4 mul, 1 rcp
}

float Vis_SmithJointApprox( float Roughness, float NoV, float NoL )
{
	float a =  Roughness * Roughness;
	float Vis_SmithV = NoL * ( NoV * ( 1 - a ) + a );
	float Vis_SmithL = NoV * ( NoL * ( 1 - a ) + a );
	return 0.5 * rcp( Vis_SmithV + Vis_SmithL );
}

float3 F_Schlick( float3 SpecularColor, float VoH )
{
	float Fc = pow( 1 - VoH, 5 );							// 1 sub, 3 mul
	//float Fc = exp2( (-5.55473 * VoH - 6.98316) * VoH );	// 1 mad, 1 mul, 1 exp
	//return Fc + (1 - Fc) * SpecularColor;					// 1 add, 3 mad
	
	// Anything less than 2% is physically impossible and is instead considered to be shadowing
	return saturate( 50.0 * SpecularColor.g ) * Fc + (1 - Fc) * SpecularColor;
	
}

float3 Diffuse_Lambert( float3 DiffuseColor )
{
	return DiffuseColor * (1 / PI);
}

float4 main(PS_INPUT input) : SV_TARGET0
{
    // Calculate the normal, including the information in the bump map
	float4 bumpSample = BumpConstant * (BumpTexture.Sample(g_samPoint, input.UV) * 2.0f - 1.0f);
    float3 bumpNrm = normalize(((bumpSample.x * input.Tangent) + (bumpSample.y * input.Binormal) + (bumpSample.z * input.Nrm)).xyz);

	float3 N = input.WNrm.xyz;
	//N += bumpSample.xzy;

    // Calculate the diffuse light component with the bump map normal
	float3 light = normalize(float3(-0.5773502691896257f, -0.5773502691896257f, 0.5773502691896257f));

	float3 vEye = View.xyz;
	float3 V = normalize(mul(vEye - input.WPos.xyz, (float3x3)WorldInvTranspose));
	float3 L = normalize(mul(g_LightPos - input.WPos.xyz, (float3x3)WorldInvTranspose));
	float3 H = normalize(V + L);

	float NoL = saturate( dot(N, L) );
	float NoV = abs(dot(N,V)) + 1e-5;
	float NoH = saturate(dot(N,H));
	float VoH = saturate(dot(V,H));

    float3 vColour = float3(1.0f, 1.0f, 1.0f);

    const float fRoughness = 0.05f;
	const float fEnergy = 0.5f;
    
	float3 SpecularColour = float3(0.2f, 0.2f, 0.2f);

	float D = D_GGX( fRoughness, NoH ) * fEnergy;
	float Vis = Vis_SmithJointApprox( fRoughness, NoV, NoL );
	float3 F = F_Schlick( SpecularColour, VoH );


	float4 vTextureColour = DiffuseTexture.Sample(g_samPoint, input.UV) * input.Colour;


	//return float4(Diffuse_Lambert(vTextureColour.xyz) + (D * Vis * F), 1.0f);
	
    /*float4 vIntensity = float4(ApproximateSpecularIBL(vColour, fRoughness, bumpNormal, vEye - input.WPos.xyz), 1.0f); */
     
	float diffuseIntensity = dot(normalize(mul(L, (float3x3)WorldInvTranspose)), N);
    if(diffuseIntensity < 0)
	{
        diffuseIntensity = 0;
	}
    
     
    //return saturate(vTextureColour * diffuseIntensity)/* + vIntensity)*/;
    return vTextureColour;
}