struct PS_INPUT
{
    float4 ProjectedPos : SV_POSITION;
    float ClipDepth : TEXCOORD0;	
    float Depth : TEXCOORD1;		
};

// DPDepth-pixel-shader
float main(PS_INPUT In) : SV_TARGET0
{
	// clipping
	clip(In.ClipDepth);
	
    return In.Depth;
}