cbuffer RarelyModified
{
    float Intensity : INTENSITY;
    float Roughness : ROUGHNESS;
    float Specular : SPECULAR;
    float4 LightPosition : VIEW;
};


SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

Texture2D DiffuseTexture;

struct PS_INPUT
{
    float4 Pos		: SV_POSITION;
    float4 WPos		: POSITION;
    float4 WNrm		: NORMAL;
    float2 UV		: TEXCOORD1;
};

struct PS_OUTPUT
{
    float4 Position : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float Flux : SV_TARGET2;
    float4 Diffuse : SV_TARGET3;
    float Roughness : SV_TARGET4;
};

#define PI 3.14159265f

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT)0;
    output.Position = input.WPos;
    output.Normal = float4(normalize(input.WNrm.xyz), 0.0f);
    
    float3 ToLight = (LightPosition.xyz - input.WPos.xyz);
    
    output.Flux = Intensity / (dot(ToLight, ToLight) + 1.0f);
    output.Diffuse = float4(DiffuseTexture.Sample(g_samPoint, input.UV).xyz, Specular); 
    output.Roughness = Roughness;

    return output;
}