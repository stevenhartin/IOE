#pragma once

#include <cstdint>

#include <IOE/IOECore/IOEDefines.h>

//////////////////////////////////////////////////////////////////////////

struct ID3D11DeviceContext;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	enum class EBufferMapType;
	class IOETexture;

	//////////////////////////////////////////////////////////////////////////

	struct IOETextureSource
	{
		void *pSrc;
		std::uint8_t uPixelSizeBytes;
		std::uint8_t uNumMipMaps;
	};

	//////////////////////////////////////////////////////////////////////////

	interface_class IOETexture_PlatformBase
	{
	public:
		IOETexture_PlatformBase(IOETexture & rTexture)
			: m_rTexture(rTexture)
		{
		}

	protected:
		IOETexture &GetTexture()
		{
			return m_rTexture;
		}
		const IOETexture &GetTexture() const
		{
			return m_rTexture;
		}

		virtual bool Setup(IOETextureSource * pSource /*=nullptr*/) = 0;
		virtual void Cleanup() = 0;

		virtual bool MapBuffer(EBufferMapType eMapType, void *&rpData,
							   std::uint32_t uSubresourceID = 0) = 0;
		virtual bool UnmapBuffer(std::uint32_t uSubresourceID = 0) = 0;

		virtual void OnResize() = 0;

		virtual void LoadFromMemory(
			std::uint8_t * pMemoryAddress, std::uint32_t uWidth,
			std::uint32_t uHeight, std::uint32_t uPixelByteWidth = 4) = 0;
		virtual void CopyTexture(IOETexture & rhs) = 0;

		virtual void SetDebugName(const std::string &szName) = 0;

	private:
		IOETexture &m_rTexture;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE