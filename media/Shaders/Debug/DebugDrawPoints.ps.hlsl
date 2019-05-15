struct PS_INPUT
{
    float4 Pos		: SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET0
{
    return float4(1.0f, 0.0, 0.0f, 1.0f);
}