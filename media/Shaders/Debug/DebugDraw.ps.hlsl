struct PS_INPUT
{
    float4 Pos		: SV_POSITION;
    float4 Colour   : COLOR;
};

static const float BumpConstant = 1.0f;
static const float PI = 3.14159265f;

float4 main(PS_INPUT input) : SV_TARGET0
{
    return input.Colour;
}