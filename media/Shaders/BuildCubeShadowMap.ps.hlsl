struct PS_INPUT
{
    float4 ProjectedPos : SV_POSITION;
    float Depth : TEXCOORD2;		
};

// DPDepth-pixel-shader
float main(PS_INPUT In) : SV_TARGET0
{	
	return In.Depth;
}