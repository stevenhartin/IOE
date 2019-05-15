RWTexture3D<float4> DestBuffer : register(u0);

#if (IOE_DEBUG == 1)
    RWTexture2D<float4> RenderTarget : register(u1);
    RWTexture2D<uint> DepthTarget : register(u2);
#endif

Texture3D SrcBuffer : register(t0);

cbuffer globals : register(b0)
{
	float OctreeSize;
	float Offset;
};

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint3 DestIndex = DTid;
	uint3 SrcIndex = DTid * (uint)Offset;

	float4 SrcColour0 = (SrcBuffer[SrcIndex + uint3(0, 0, 0)]);
	float4 SrcColour1 = (SrcBuffer[SrcIndex + uint3(1, 0, 0)]);
	float4 SrcColour2 = (SrcBuffer[SrcIndex + uint3(0, 1, 0)]);
	float4 SrcColour3 = (SrcBuffer[SrcIndex + uint3(1, 1, 0)]);
	float4 SrcColour4 = (SrcBuffer[SrcIndex + uint3(0, 0, 1)]);
	float4 SrcColour5 = (SrcBuffer[SrcIndex + uint3(1, 0, 1)]);
	float4 SrcColour6 = (SrcBuffer[SrcIndex + uint3(0, 1, 1)]);
	float4 SrcColour7 = (SrcBuffer[SrcIndex + uint3(1, 1, 1)]);

	float4 OutColour = float4(0.0f, 0.0f, 0.0f, 0.0f);

#if DIRECTION == 0
		// +X
		OutColour =
			SrcColour0 + SrcColour1 * (1.0 - SrcColour0.w)
			+ SrcColour2 + SrcColour3 * (1.0 - SrcColour2.w)
			+ SrcColour4 + SrcColour5 * (1.0 - SrcColour4.w)
			+ SrcColour6 + SrcColour7 * (1.0 - SrcColour6.w);
#elif DIRECTION == 1
		// -X
		OutColour =
			SrcColour1 + SrcColour0 * (1.0 - SrcColour1.w)
			+ SrcColour3 + SrcColour2 * (1.0 - SrcColour3.w)
			+ SrcColour5 + SrcColour4 * (1.0 - SrcColour5.w)
			+ SrcColour7 + SrcColour6 * (1.0 - SrcColour7.w);
		
#elif DIRECTION == 2
		// +Y
		OutColour =
			SrcColour0 + SrcColour2 * (1.0 - SrcColour0.w)
			+ SrcColour1 + SrcColour3 * (1.0 - SrcColour1.w)
			+ SrcColour4 + SrcColour6 * (1.0 - SrcColour4.w)
			+ SrcColour5 + SrcColour7 * (1.0 - SrcColour5.w);
#elif DIRECTION == 3
		// -Y
		OutColour =
			SrcColour2 + SrcColour0 * (1.0 - SrcColour2.w)
			+ SrcColour3 + SrcColour1 * (1.0 - SrcColour3.w)
			+ SrcColour6 + SrcColour4 * (1.0 - SrcColour6.w)
			+ SrcColour7 + SrcColour5 * (1.0 - SrcColour7.w);
#elif DIRECTION == 4
		// +Z
		OutColour =
			SrcColour0 + SrcColour4 * (1.0 - SrcColour0.w)
			+ SrcColour1 + SrcColour5 * (1.0 - SrcColour1.w)
			+ SrcColour2 + SrcColour6 * (1.0 - SrcColour2.w)
			+ SrcColour3 + SrcColour7 * (1.0 - SrcColour3.w);
#elif DIRECTION == 5
		// -Z
		OutColour =
			SrcColour4 + SrcColour0 * (1.0 - SrcColour4.w)
			+ SrcColour5 + SrcColour1 * (1.0 - SrcColour5.w)
			+ SrcColour6 + SrcColour2 * (1.0 - SrcColour6.w)
			+ SrcColour7 + SrcColour3 * (1.0 - SrcColour7.w);
#else
# error shouldn't be here
#endif
		OutColour *= 0.25f;

		DestBuffer[DestIndex] = (OutColour);

#if (IOE_DEBUG == 1)
		DeviceMemoryBarrierWithGroupSync();
		if (OutColour.a > 0.1f)
		{
#if DIRECTION == 0
			uint2 UV = uint2(DestIndex.z, (uint)(OctreeSize - 1) - DestIndex.y);
			uint Depth = DestIndex.x;
#elif DIRECTION == 1
			uint2 UV = uint2((uint)(OctreeSize - 1) - DestIndex.z, (uint)(OctreeSize - 1) - DestIndex.y);
			uint Depth = (uint)(OctreeSize - 1) - DestIndex.x;
#elif DIRECTION == 2
			uint2 UV = uint2(DestIndex.x, (uint)(OctreeSize - 1) - DestIndex.z);
			uint Depth = DestIndex.y;
#elif DIRECTION == 3
			uint2 UV = uint2(DestIndex.x, DestIndex.z);
			uint Depth = (uint)(OctreeSize - 1) - DestIndex.y;
#elif DIRECTION == 4
			uint2 UV = uint2((uint)(OctreeSize - 1) - DestIndex.x, (uint)(OctreeSize - 1) - DestIndex.y);
			uint Depth = DestIndex.z;
#else
			uint2 UV = uint2(DestIndex.x, (uint)(OctreeSize - 1) - DestIndex.y);
			uint Depth = (uint)(OctreeSize - 1) - DestIndex.z;
#endif
			//Depth = (uint)(OctreeSize - 1) - Depth;
			if (DepthTarget[UV] > Depth)
			{
				//RenderTarget[UV] = float4(Depth / OctreeSize, Depth / OctreeSize, Depth / OctreeSize, 1.0f)/*OutColour*/;
				RenderTarget[UV] = OutColour;
				DepthTarget[UV] = Depth;
			}
		}
		DeviceMemoryBarrierWithGroupSync();
#endif
}