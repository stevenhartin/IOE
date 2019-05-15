struct PSInput
{
    float4 Pos		: SV_POSITION;
    float4 Colour	: COLOR;
    float2 UV		: TEXCOORD0;
};

Texture2D GlyphSampler;

SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 main(PSInput input) : SV_TARGET0
{
    float2 UV = input.UV;

    float4 vTextureColour = GlyphSampler.Sample(g_samPoint, UV);

    return vTextureColour * input.Colour;
}
