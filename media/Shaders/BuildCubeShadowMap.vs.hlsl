// DPDepth
cbuffer GlobalSettings
{
    float4x4 WorldViewProj : WORLDVIEWPROJ;
    float4x4 WorldInvTranspose : WORLDINVTRANSPOSE;
}

cbuffer PassSettings
{
    float4 LightPos;
}

struct VS_INPUT
{
    float4 Pos      : POSITION;
    float4 Nrm      : NORMAL;
    float4 Colour   : COLOR;
    float2 UV	    : TEXCOORD0;
    float4 Tangent  : TEXCOORD1;
    float4 Binormal : TEXCOORD2;
};

struct PS_INPUT
{
    float4 ProjectedPos : SV_POSITION;
    float Depth : TEXCOORD2;		
};


// DPDepth-vertex-shader
PS_INPUT main(VS_INPUT In)
{
    PS_INPUT Out;	
    
    // transform vertex to DP-space
    Out.ProjectedPos = mul(float4(In.Pos.xyz, 1.0f), WorldViewProj);

    Out.Depth = length(LightPos.xyz - In.Pos.xyz);
   
    return Out;
}