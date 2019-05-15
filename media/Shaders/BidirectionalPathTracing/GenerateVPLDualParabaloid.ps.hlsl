struct PS_INPUT
{
    float4 ProjectedPos : SV_POSITION;
    float Intensity     : TEXCOORD0;
    float ClipDepth     : TEXCOORD1;	
    float Depth         : TEXCOORD2;
	float3 ToLight		: NORMAL;			
};

struct PS_OUTPUT
{
    float Colour : SV_TARGET0;
};

PS_OUTPUT main(PS_INPUT In)
{
    // clipping
	clip(In.ClipDepth);
    
	PS_OUTPUT output = (PS_OUTPUT)0;
    
    // For a point light,
    // I' = I / (4 pi r^2)
    output.Colour = (In.Intensity / (dot(In.ToLight, In.ToLight) + 1.0f));
    //output.Colour = 1.0f;

	return output;
}