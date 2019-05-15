// DPDepth
cbuffer GlobalSettings
{
    float4x4 WorldViewProj : WORLDVIEWPROJ;
    float4x4 WorldInvTranspose : WORLDINVTRANSPOSE;
    float4 View : EYE;
}

cbuffer PassSettings
{
    float Dir;
    float Far;
    float Near;
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
    float ClipDepth : TEXCOORD0;	
    float Depth : TEXCOORD1;
};


// DPDepth-vertex-shader
PS_INPUT main(VS_INPUT In)
{
    PS_INPUT Out = (PS_INPUT)0;	
    
    // transform vertex to DP-space
	Out.ProjectedPos = mul(float4(In.Pos.xyz, 1.0f), WorldViewProj);
    Out.ProjectedPos /= Out.ProjectedPos.w;

    // for the back-map z has to be inverted
	Out.ProjectedPos.z *= Dir;
    
    float fLength = length(Out.ProjectedPos.xyz);
    
    // because the origin is at 0 the proj-vector
    // matches the vertex-position
    
    // normalize
    Out.ProjectedPos /= fLength;
        
    // save for clipping 	
    Out.ClipDepth = Out.ProjectedPos.z;
    
    // calc "normal" on intersection, by adding the 
    // reflection-vector(0,0,1) and divide through 
    // his z to get the texture coords
    Out.ProjectedPos.x /= Out.ProjectedPos.z + 1.0f;
    Out.ProjectedPos.y /= Out.ProjectedPos.z + 1.0f;
    
    // set z for z-buffering and neutralize w
    Out.ProjectedPos.z = ((fLength - Near) / (Far - Near));
    Out.ProjectedPos.w = 1.0f;
    
    // DP-depth
    Out.Depth = Out.ProjectedPos.z;
    
    return Out;
}