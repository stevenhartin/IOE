// input
struct GSInput
{
	float4 Pos : SV_POSITION;
	float3 UV : TEXCOORD0;
};

// output
struct GSOutput
{
	float4 Pos: SV_POSITION;
	float3 UV : TEXCOORD0;
	float3 offset : TEXCOORD1;
};

cbuffer Globals
{
	float4x4 WorldViewProj;
};

cbuffer RarelyUsed
{
	float cubeScale;
}

GSOutput FillOutput(float3 centerPos, float3 offset, float3 UV)
{
	GSOutput output = (GSOutput)0;
	output.Pos = mul(float4(centerPos + offset * cubeScale, 1.0f), WorldViewProj);
	output.UV = UV;
	output.offset = offset;
	return output;
}

static const float fMult = 0.999f;

// point -> cube
[maxvertexcount(24)]
void main(point GSInput gin[1], 
            inout TriangleStream<GSOutput> triStream)
{
	float4 centerPos = gin[0].Pos;

	GSOutput output = (GSOutput)0;
	output.UV = gin[0].UV;
	
	// throw away off-screen cubes to save some performance
	/*float4 centerNDCPos = mul(float4(centerPos.xyz, 1.0f), WorldViewProj);
	if (centerNDCPos.x < -centerNDCPos.w
		|| centerNDCPos.x > centerNDCPos.w
		|| centerNDCPos.y < -centerNDCPos.w
		|| centerNDCPos.y > centerNDCPos.w
		|| centerNDCPos.z < -centerNDCPos.w
		|| centerNDCPos.z > centerNDCPos.w)
		return;*/
	
	// All CCW RH coordinate system
	// -X
	{
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f, 0.5f * fMult, 0.5f * fMult), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f, 0.5f * fMult, -0.5f * fMult), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f, -0.5f * fMult, 0.5f * fMult), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f, -0.5f * fMult, -0.5f * fMult), gin[0].UV));
	}
	triStream.RestartStrip();
	// +X
	{
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f, 0.5f * fMult, -0.5f * fMult), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f, 0.5f * fMult, 0.5f * fMult), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f, -0.5f * fMult, -0.5f * fMult), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f, -0.5f * fMult, 0.5f * fMult), gin[0].UV));
	}
	triStream.RestartStrip();
	// -Y
	{
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, -0.5f, -0.5f * fMult), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, -0.5f, -0.5f * fMult), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, -0.5f, 0.5f * fMult), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, -0.5f, 0.5f * fMult), gin[0].UV));
	}
	triStream.RestartStrip();
	// +Y
	{
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, 0.5f, -0.5f * fMult), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, 0.5f, -0.5f * fMult), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, 0.5f, 0.5f * fMult), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, 0.5f, 0.5f * fMult), gin[0].UV));
	}
	triStream.RestartStrip();
	// -Z
	{
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, -0.5f * fMult, -0.5f), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, 0.5f * fMult, -0.5f), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, -0.5f * fMult, -0.5f), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, 0.5f * fMult, -0.5f), gin[0].UV));
	}
	triStream.RestartStrip();
	// +Z
	{
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, -0.5f * fMult, 0.5f), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(0.5f * fMult, 0.5f * fMult, 0.5f), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, -0.5f * fMult, 0.5f), gin[0].UV));
		triStream.Append(FillOutput(centerPos.xyz, float3(-0.5f * fMult, 0.5f * fMult, 0.5f), gin[0].UV));
	}
	triStream.RestartStrip();
}
