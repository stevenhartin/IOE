#include <fstream>
#include <string>
#include <memory>
#include <cstdint>

#include <IOE/IOEExceptions/IOEException_Enum.h>
#include <IOE/IOEExceptions/IOEException_Renderer.h>

#include "IOETexture.h"

//////////////////////////////////////////////////////////////////////////

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

using std::int8_t;
using std::int16_t;
using std::int32_t;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	using namespace IOE::Core::Algorithm;

	//////////////////////////////////////////////////////////////////////////

	IOETexture::IOETexture()
		: m_tPlatform(*this)
		, m_eTextureType(ETextureType::Unknown)
		, m_eTextureFormat(ETextureFormat::Unknown)
		, m_eBufferAccess(ECPUAccess::GPUReadWrite)
		, m_eBufferDescription(EBufferDescription::Unknown)
		, m_eCubeFace(ECubeFace::PositiveX)
		, m_fWidth(0.0f)
		, m_fHeight(0.0f)
		, m_fDepth(0.0f)
		, m_bDefaultRenderTarget(false)
		, m_nNumMips(0)
		, m_nMipToMap(0)
		, m_bCreateView(true)
		, m_nArrayLength(1)
		, m_nArrayFirstIndex(0)
		, m_nArrayToMapCount(1)
		, m_bImplicitCounter(false)
		, m_bAllowRawValues(false)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	IOETexture::IOETexture(ETextureFormat eFormat, float fWidth,
						   std::int32_t nArrayCount /*=1*/,
						   bool bDefaultRenderTarget /*=false*/)
		: IOETexture()
	{
		m_nArrayLength = m_nArrayToMapCount = nArrayCount;
		m_eTextureFormat	   = eFormat;
		m_fWidth			   = fWidth;
		m_bDefaultRenderTarget = bDefaultRenderTarget;
		m_eTextureType = IsTextureArray() ? ETextureType::Texture1DArray
										  : ETextureType::Texture1D;
	}

	//////////////////////////////////////////////////////////////////////////

	IOETexture::IOETexture(ETextureFormat eFormat, float fWidth, float fHeight,
						   std::int32_t nArrayCount /*=1*/,
						   bool bDefaultRenderTarget /*=false*/)
		: IOETexture()
	{
		m_nArrayLength = m_nArrayToMapCount = nArrayCount;
		m_eTextureFormat	   = eFormat;
		m_fWidth			   = fWidth;
		m_fHeight			   = fHeight;
		m_bDefaultRenderTarget = bDefaultRenderTarget;
		m_eTextureType = IsTextureArray() ? ETextureType::Texture2DArray
										  : ETextureType::Texture2D;
	}

	//////////////////////////////////////////////////////////////////////////

	IOETexture::IOETexture(ETextureFormat eFormat, float fWidth, float fHeight,
						   float fDepth, bool bDefaultRenderTarget /*=false*/)
		: IOETexture()
	{
		m_eTextureFormat	   = eFormat;
		m_fWidth			   = fWidth;
		m_fHeight			   = fHeight;
		m_fDepth			   = fDepth;
		m_bDefaultRenderTarget = bDefaultRenderTarget;
		m_eTextureType		   = ETextureType::Texture3D;
	}

	//////////////////////////////////////////////////////////////////////////

	IOETexture::IOETexture(ETextureFormat eFormat, float fWidth, float fHeight,
						   ETextureType eType, int32_t nArrayCount /*=1*/)
		: IOETexture()
	{
		m_eTextureFormat = eFormat;
		m_fWidth		 = fWidth;
		m_fHeight		 = fHeight;
		m_eTextureType   = eType;
		m_nArrayLength = m_nArrayToMapCount = nArrayCount;
	}

	//////////////////////////////////////////////////////////////////////////

	IOETexture::~IOETexture()
	{
		Cleanup();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETexture::Resize(float fWidth)
	{
		IOE_ASSERT(m_eTextureType == ETextureType::Texture1D ||
					   m_eTextureType == ETextureType::Texture1DArray,
				   "Must set all dimension!");
		m_fWidth = fWidth;
		m_tPlatform.OnResize();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETexture::Resize(float fWidth, float fHeight)
	{
		IOE_ASSERT(m_eTextureType == ETextureType::Texture2D ||
					   m_eTextureType == ETextureType::Texture2DArray,
				   "Must set all dimension!");
		m_fWidth  = fWidth;
		m_fHeight = fHeight;
		m_tPlatform.OnResize();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETexture::Resize(float fWidth, float fHeight, float fDepth)
	{
		IOE_ASSERT(m_eTextureType == ETextureType::Texture3D,
				   "Must set all dimension!");
		m_fWidth  = fWidth;
		m_fHeight = fHeight;
		m_fDepth  = fDepth;
		m_tPlatform.OnResize();
	}

	//////////////////////////////////////////////////////////////////////////

	std::shared_ptr<IOETexture>
	IOETextureManager::LoadFromFile(const std::wstring &szPath)
	{
		auto pTexture(GetTexture(szPath));
		if (pTexture)
		{
			return pTexture;
		}

		std::ifstream tStream(szPath, std::ios::in | std::ios::binary);
		if (!tStream.is_open())
		{
			THROW_IOE_RENDERER_EXCEPTION_VARGS("Failed to open path: '%ls'",
											   E_ERROR_FAILED_TO_LOAD_TEXTURE,
											   szPath.c_str());
		}

		tStream.seekg(0, std::ios::end);
		std::size_t nNumBytes(tStream.tellg());
		tStream.seekg(0, std::ios::beg);
		if (nNumBytes == 0)
		{
			THROW_IOE_RENDERER_EXCEPTION_VARGS("File '%ls' is empty",
											   E_ERROR_FAILED_TO_LOAD_TEXTURE,
											   szPath.c_str());
		}

		auto pBuffer(std::make_unique<std::uint8_t[]>(nNumBytes));

		tStream.read(reinterpret_cast<char *>(pBuffer.get()), nNumBytes);

		tStream.close();

		std::wstring szExtension(GetExtension(szPath));

		if (szExtension == L"tga")
		{
			pTexture = LoadFromTGA(pBuffer.get());
		}
		else
		{
			THROW_IOE_RENDERER_EXCEPTION_VARGS(
				"Extension '%ls' is an unsupported texture format",
				E_ERROR_FAILED_TO_LOAD_TEXTURE, szExtension.c_str())
		}

		m_arrTextures.emplace_back(ToLower(szPath), pTexture);

		return pTexture;
	}

	//////////////////////////////////////////////////////////////////////////

	std::shared_ptr<IOETexture>
	IOETextureManager::LoadFromTGA(std::uint8_t *pMemory)
	{
#define READ_VARIABLE(VarType, VarName)                                       \
	VarType VarName(CopyAndMovePtr<VarType>(pMemory))

		// Read header info
		READ_VARIABLE(
			uint8_t,
			uImageIDLength); // Length of the Image ID field (user data)
		READ_VARIABLE(uint8_t, uColourMapType); // Whether a colour map is
												// included (0 = none, 1 =
												// present, 2 - 127 =
												// truevision, 128 - 255 =
												// developer use)
		READ_VARIABLE(uint8_t, uImageType); // Compression and colour types (0
											// = no image data, 1 = raw colour,
											// 2 = raw true colour, 3 = raw
											// greyscale, 9 = run-length
											// encoded colour, 10 = run-length
											// encoded true colour, 11 =
											// run-length encoded greyscale)

		// Read colour map specification
		READ_VARIABLE(uint16_t, uFirstEntryOffset); // The offset into the
													// colour map table of the
													// specification
		READ_VARIABLE(uint16_t, uNumPixels); // Number of colour map entries
		READ_VARIABLE(uint8_t,
					  uColourMapPixelBitSize); // Colour map pixel bit size

		// Image specification
		READ_VARIABLE(uint16_t, uXOrigin); // absolute coordinate of lower-left
										   // corner for displays where origin
										   // is at lower-left
		READ_VARIABLE(uint16_t, uYOrigin); // As above
		READ_VARIABLE(uint16_t, uWidth);   // Width of texture in pixels
		READ_VARIABLE(uint16_t, uHeight);  // Height of texture in pixels
		READ_VARIABLE(uint8_t, uPixelDepth);	  // Size of each pixel in bits
		READ_VARIABLE(uint8_t, uImageDescriptor); // bits 3 - 0 give alpha
												  // channel depth, bits 5 - 4
												  // give direction

		IOE_ASSERT(uPixelDepth % 8 == 0,
				   "Pixel depth must be evenly divisible by 8 so it fits into "
				   "whole bytes");

		void *pUserData(GetPointerAndMovePtr(pMemory, uImageIDLength));
		void *pColourMapData(GetPointerAndMovePtr(pMemory, uNumPixels *
													  uColourMapPixelBitSize));
		uint8_t *pImageData(static_cast<uint8_t *>(GetPointerAndMovePtr(
			pMemory, (uPixelDepth / 8) * uWidth * uHeight)));

#undef READ_VARIABLE

		ETextureFormat eFormat(ETextureFormat::B8G8R8A8_UNORM);
		if (uImageType != 2 && uImageType != 3)
		{
			THROW_IOE_RENDERER_EXCEPTION(
				"File colour mode is invalid, expects 24 bit or 32 bit colour "
				"mode!",
				E_ERROR_FAILED_TO_LOAD_TEXTURE)
		}

		// Pixel buffer is in BGRA format
		auto pPixelBuffer = std::make_unique<uint32_t[]>(uWidth * uHeight);

		for (uint16_t uY(0); uY < uHeight; ++uY) // for every row
		{
			for (uint16_t uX(0); uX < uWidth; ++uX) // for every col
			{
				uint32_t uOffset(uY * uWidth + uX);

				if (uImageType == 2)
				{
					// Expand out if 24 bits. For every 3 bytes, we store into
					// a 4 byte integer and explicitly
					// set the alpha to 0xFF
					memcpy(&(pPixelBuffer.get()[uOffset]),
						   &pImageData[uOffset * 3], 3);
					pPixelBuffer.get()[uOffset] |= 0xFF000000;
				}
				else
				{
					memcpy(&(pPixelBuffer.get()[uOffset]),
						   &pImageData[uOffset * 4], 4);
				}
			}
		}

		auto pTexture(std::make_shared<IOETexture>(
			eFormat, static_cast<float>(uWidth), static_cast<float>(uHeight)));

		pTexture->SetBufferDescription(EBufferDescription::ShaderResource);

		IOETextureSource tSource;
		tSource.pSrc			= pPixelBuffer.get();
		tSource.uPixelSizeBytes = 4;
		tSource.uNumMipMaps		= 1;
		pTexture->Setup(&tSource);

		return pTexture;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOETextureManager::IsLoaded(const std::wstring &szPath) const
	{
		std::wstring szLowerPath(ToLower(szPath));
		for (auto &pPtr : m_arrTextures)
		{
			if (pPtr.szTexturePath == szLowerPath)
			{
				return true;
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	std::shared_ptr<IOETexture>
	IOETextureManager::GetTexture(const std::wstring &szPath)
	{
		std::wstring szLowerPath(ToLower(szPath));
		for (auto &pPtr : m_arrTextures)
		{
			if (pPtr.szTexturePath == szLowerPath)
			{
				return pPtr.pTexture;
			}
		}
		return std::shared_ptr<IOETexture>();
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE