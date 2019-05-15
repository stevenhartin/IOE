struct PS_INPUT
{
    float4 Pos		: SV_POSITION;
    float Intensity : TEXCOORD0;
};

struct PS_OUTPUT
{
    float Colour : SV_TARGET0;
};

PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;
    output.Colour = input.Intensity;
	return output;
}