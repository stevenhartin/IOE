#pragma once

//////////////////////////////////////////////////////////////////////////

#include <cstdint>

#include <IOE/IOECore/IOEHeaders.h>
#include <IOE/IOECore/IOEManager.h>
#include "IOEBuffer.h"
#include RPI_INCLUDE(IOETexture_Platform.h)

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	enum class ETextureType
	{
		Texture1D,
		Texture2D,
		Texture3D,
		TextureCube,
		Texture1DArray,
		Texture2DArray,
		TextureCubeArray,
		GenericBuffer,

		Unknown,
	};

	//////////////////////////////////////////////////////////////////////////

	enum class ETextureFormat
	{
		R32G32B32A32_TYPELESS,
		R32G32B32A32_FLOAT,
		R32G32B32A32_UINT,
		R32G32B32A32_SINT,
		R32G32B32_TYPELESS,
		R32G32B32_FLOAT,
		R32G32B32_UINT,
		R32G32B32_SINT,
		R16G16B16A16_TYPELESS,
		R16G16B16A16_FLOAT,
		R16G16B16A16_UNORM,
		R16G16B16A16_UINT,
		R16G16B16A16_SNORM,
		R16G16B16A16_SINT,
		R32G32_TYPELESS,
		R32G32_FLOAT,
		R32G32_UINT,
		R32G32_SINT,
		R32G8X24_TYPELESS,
		D32_FLOAT_S8X24_UINT,
		R32_FLOAT_X8X24_TYPELESS,
		X32_TYPELESS_G8X24_UINT,
		R10G10B10A2_TYPELESS,
		R10G10B10A2_UNORM,
		R10G10B10A2_UINT,
		R11G11B10_FLOAT,
		R8G8B8A8_TYPELESS,
		R8G8B8A8_UNORM,
		R8G8B8A8_UNORM_SRGB,
		R8G8B8A8_UINT,
		R8G8B8A8_SNORM,
		R8G8B8A8_SINT,
		R16G16_TYPELESS,
		R16G16_FLOAT,
		R16G16_UNORM,
		R16G16_UINT,
		R16G16_SNORM,
		R16G16_SINT,
		R32_TYPELESS,
		D32_FLOAT,
		R32_FLOAT,
		R32_UINT,
		R32_SINT,
		R24G8_TYPELESS,
		D24_UNORM_S8_UINT,
		R24_UNORM_X8_TYPELESS,
		X24_TYPELESS_G8_UINT,
		R8G8_TYPELESS,
		R8G8_UNORM,
		R8G8_UINT,
		R8G8_SNORM,
		R8G8_SINT,
		R16_TYPELESS,
		R16_FLOAT,
		D16_UNORM,
		R16_UNORM,
		R16_UINT,
		R16_SNORM,
		R16_SINT,
		R8_TYPELESS,
		R8_UNORM,
		R8_UINT,
		R8_SNORM,
		R8_SINT,
		A8_UNORM,
		R1_UNORM,
		B8G8R8A8_UNORM,

		Unknown
	};

	//////////////////////////////////////////////////////////////////////////

	// Add all different shader resource texture types here. These determine
	// which samplers to send to.
	enum class EShaderResourceTextureInterpretation
	{
		Unknown,
		Diffuse,
		Bump,
	};

	//////////////////////////////////////////////////////////////////////////

	enum class ECubeFace
	{
		PositiveX,
		NegativeX,
		PositiveY,
		NegativeY,
		PositiveZ,
		NegativeZ,
		All,

		NumFaces = ECubeFace::All
	};

	//////////////////////////////////////////////////////////////////////////

	class IOETexture
	{
	private:
		IOETexture();

	public:
		// Texture1D
		IOETexture(ETextureFormat eFormat, float fWidth,
				   int32_t nArrayCount = 1, bool bDefaultRenderTarget = false);
		// Texture2D
		IOETexture(ETextureFormat eFormat, float fWidth, float fHeight,
				   int32_t nArrayCount = 1, bool bDefaultRenderTarget = false);
		// Texture3D
		IOETexture(ETextureFormat eFormat, float fWidth, float fHeight,
				   float fDepth, bool bDefaultRenderTarget = false);

		// TextureCube
		IOETexture(ETextureFormat eFormat, float fWidth, float fHeight,
				   ETextureType eType, int32_t nArrayCount = 1);

		~IOETexture();

		FORCEINLINE void SetDebugName(const std::string &szName)
		{
			m_tPlatform.SetDebugName(szName);
		}

	public:
		FORCEINLINE void SetTextureType(ETextureType eTextureType)
		{
			m_eTextureType = eTextureType;
		}
		FORCEINLINE ETextureType GetTextureType() const
		{
			return m_eTextureType;
		}
		FORCEINLINE ETextureFormat GetTextureFormat() const
		{
			return m_eTextureFormat;
		}
		FORCEINLINE bool IsDefaultRenderTarget() const
		{
			return m_bDefaultRenderTarget;
		}
		FORCEINLINE bool Setup(IOETextureSource *pTextureSource = nullptr)
		{
			return m_tPlatform.Setup(pTextureSource);
		}
		FORCEINLINE void Cleanup()
		{
			m_tPlatform.Cleanup();
		}

	public:
		void Resize(float fWidth);
		void Resize(float fWidth, float fHeight);
		void Resize(float fWidth, float fHeight, float fDepth);

		FORCEINLINE void SetCubeFace(ECubeFace eFace)
		{
			m_eCubeFace = eFace;
		}
		FORCEINLINE ECubeFace GetCubeFace() const
		{
			return m_eCubeFace;
		}

		void SetCreateView(bool bValue)
		{
			m_bCreateView = bValue;
		}
		bool GetCreateView() const
		{
			return m_bCreateView;
		}

	public:
		FORCEINLINE void SetBufferAccess(ECPUAccess eAccess)
		{
			m_eBufferAccess = eAccess;
		}
		FORCEINLINE ECPUAccess GetBufferAccess() const
		{
			return m_eBufferAccess;
		}

		FORCEINLINE EBufferDescription::Enum GetBufferDescription() const
		{
			return m_eBufferDescription;
		}
		FORCEINLINE void SetBufferDescription(std::uint32_t eDescription)
		{
			m_eBufferDescription = (EBufferDescription::Enum)eDescription;
		}

		FORCEINLINE void SetTextureInterpretationType(
			EShaderResourceTextureInterpretation eType)
		{
			m_eTextureInterpretation = eType;
		}
		FORCEINLINE EShaderResourceTextureInterpretation
		GetTextureInterpretationType() const
		{
			return m_eTextureInterpretation;
		}

		FORCEINLINE IOETexture_Platform &GetPlatform()
		{
			return m_tPlatform;
		}
		FORCEINLINE const IOETexture_Platform &GetPlatform() const
		{
			return m_tPlatform;
		}

		FORCEINLINE void CopyTexture(IOETexture &rOther)
		{
			m_tPlatform.CopyTexture(rOther);
		}

		FORCEINLINE void SetHasImplicitCounter(bool bValue)
		{
			m_bImplicitCounter = bValue;
		}
		FORCEINLINE bool HasImplicitCounter() const
		{
			return m_bImplicitCounter;
		}

		FORCEINLINE void SetAllowRawValue(bool bValue)
		{
			m_bAllowRawValues = bValue;
		}
		FORCEINLINE bool CanAllowRawValues() const
		{
			return m_bAllowRawValues;
		}

	public:
		FORCEINLINE bool MapBuffer(EBufferMapType eMapType, void *&rpData,
								   std::uint32_t uSubresourceID = 0)
		{
			return m_tPlatform.MapBuffer(eMapType, rpData, uSubresourceID);
		}
		FORCEINLINE bool UnmapBuffer(std::uint32_t uSubresourceID = 0)
		{
			return m_tPlatform.UnmapBuffer(uSubresourceID);
		}

	public:
		FORCEINLINE float GetWidth() const
		{
			return m_fWidth;
		}
		FORCEINLINE float GetHeight() const
		{
			return m_fHeight;
		}
		FORCEINLINE float GetDepth() const
		{
			return m_fDepth;
		}

		FORCEINLINE bool IsTextureArray() const
		{
			return (m_eTextureType == ETextureType::Texture1DArray ||
					m_eTextureType == ETextureType::Texture2DArray ||
					m_eTextureType == ETextureType::TextureCubeArray) ||
				GetArrayLength() > 1;
		}
		FORCEINLINE int32_t GetArrayLength() const
		{
			return m_nArrayLength;
		}

		FORCEINLINE void SetFirstArrayIndexToMap(int32_t nArrayFirstIndex)
		{
			m_nArrayFirstIndex = nArrayFirstIndex;
		}
		FORCEINLINE int32_t GetFirstArrayIndexToMap() const
		{
			return m_nArrayFirstIndex;
		}

		FORCEINLINE void SetArrayCountToMap(int32_t nArrayToMapCount)
		{
			m_nArrayToMapCount = nArrayToMapCount;
		}
		FORCEINLINE int32_t GetArrayCountToMap() const
		{
			return m_nArrayToMapCount;
		}

		FORCEINLINE std::int32_t GetNumMips() const
		{
			return m_nNumMips;
		}
		FORCEINLINE void SetNumMips(std::int32_t nNumMips)
		{
			m_nNumMips = nNumMips;
		}

		FORCEINLINE std::int32_t GetMipToMap() const
		{
			return m_nMipToMap;
		}
		FORCEINLINE void SetMipToMap(std::int32_t nMipToMap)
		{
			m_nMipToMap = nMipToMap;
		}

	private:
		IOETexture_Platform m_tPlatform;
		ETextureType m_eTextureType;
		ETextureFormat m_eTextureFormat;
		ECPUAccess m_eBufferAccess;
		EBufferDescription::Enum m_eBufferDescription;
		EShaderResourceTextureInterpretation m_eTextureInterpretation;

		ECubeFace m_eCubeFace;

		float m_fWidth;
		float m_fHeight;
		float m_fDepth;

		// Number of mips to generate as part of the texture
		int32_t m_nNumMips;

		// The mip level to bind to the texture view
		int32_t m_nMipToMap;

		// Number of array indices for texture array (1D or 2D)
		int32_t m_nArrayLength;
		// First element in the texture array to map (default to 0)
		int32_t m_nArrayFirstIndex;
		// Number of elements in the texture array to map (default to all)
		int32_t m_nArrayToMapCount;

		// Create a texture view
		bool m_bCreateView;

		bool m_bDefaultRenderTarget;

		// Add a counter with the buffer (only for UAVs).
		bool m_bImplicitCounter;

		// Allow raw values for UAVs.
		bool m_bAllowRawValues;
	};

	//////////////////////////////////////////////////////////////////////////

	class IOETextureManager : public IOE::Core::IOEManager<IOETextureManager>
	{
		struct IOETextureMap
		{
			IOETextureMap(const std::wstring &szPath_,
						  std::shared_ptr<IOETexture> pTexture_)
				: szTexturePath(szPath_)
				, pTexture(pTexture_)
			{
			}

			std::wstring szTexturePath;
			std::shared_ptr<IOETexture> pTexture;
		};

	public:
		IOETextureManager()
			: IOEManager("IOETextureManager")
		{
		}

	public:
		bool IsLoaded(const std::wstring &szPath) const;
		std::shared_ptr<IOETexture> GetTexture(const std::wstring &szPath);

	public:
		std::shared_ptr<IOETexture> LoadFromFile(const std::wstring &szPath);

	private:
		std::shared_ptr<IOETexture> LoadFromTGA(std::uint8_t *pMemory);

	private:
		std::vector<IOETextureMap> m_arrTextures;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace renderer
} // namespace IOE