struct PSInput
{
    float4 Pos      : SV_POSITION;
    float2 UV	    : TEXCOORD0;
};

cbuffer Globals
{
    float LightModifier;
}

#if defined(TEXTURE_CUBE_DIRECTION)
TextureCube DiffuseTexture;
#elif defined(TEXTURE_INDEX)
Texture2DArray DiffuseTexture;
#else
Texture2D DiffuseTexture;
#endif

SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 main(PSInput input) : SV_TARGET0
{
#if defined(TEXTURE_CUBE_DIRECTION)
    float x = (1.0f - input.UV.x) * 2.0f - 1.0f;
    float y = (1.0f - input.UV.y) * 2.0f - 1.0f;
#if (TEXTURE_CUBE_DIRECTION == 0)
    float3 UV = float3(1.0f, y, x);
#elif (TEXTURE_CUBE_DIRECTION == 1)
    float3 UV = float3(-1.0f, y, -x);
#elif (TEXTURE_CUBE_DIRECTION == 2)
    float3 UV = float3(x, 1.0f, y);
#elif (TEXTURE_CUBE_DIRECTION == 3)
    float3 UV = float3(x, -1.0f, y);
#elif (TEXTURE_CUBE_DIRECTION == 4)
    float3 UV = float3(x, y, 1.0f);
#elif (TEXTURE_CUBE_DIRECTION == 5)
    float3 UV = float3(-x, y, -1.0f);
#endif
#elif defined(TEXTURE_INDEX)
    float3 UV = float3(input.UV, (float)TEXTURE_INDEX);
#else
    float2 UV = input.UV;
#endif
    float4 vTextureColour = DiffuseTexture.Sample(g_samPoint, UV);
    return vTextureColour * LightModifier;
}
