#ifndef __COMMON_DEFS_H__
#define __COMMON_DEFS_H__

static const float PI = 3.14159265f;
static const float FLT_MAX = 3.402823466e+38f;
static const float FLT_MIN = 1.175494351e-38f;
static const float Tolerance = 0.00001f;

////////////////////////////////////////////////////////////////////////////////

float4x4 CreateLookAtLH(float3 EyePosition, float3 EyeDirection, float3 UpDirection)
{
	float3 R2 = normalize(EyeDirection);

	float3 R0 = normalize(cross(UpDirection, R2));

	float3 R1 = cross(R2, R0);
	float3 NegEyePosition = -EyePosition;

	float D0 = dot(R0, NegEyePosition);
	float D1 = dot(R1, NegEyePosition);
	float D2 = dot(R2, NegEyePosition);

	float4x4 M;
	M[0] = float4(R0, D0);
	M[1] = float4(R1, D1);
	M[2] = float4(R2, D2);
	M[3] = float4(0.0f, 0.0f, 0.0f, 1.0f);

	return transpose(M);
}

////////////////////////////////////////////////////////////////////////////////

#endif // __COMMON_DEFS_H__