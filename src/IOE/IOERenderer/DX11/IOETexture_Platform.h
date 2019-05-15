#pragma once

#include <cstdint>
#include <d3d11.h>

#include "../IOETexture_PlatformBase.h"
#include "IOEComSharedPtr.h"

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOETexture;
	enum class EBufferMapType;
	enum class ETextureFormat;

	//////////////////////////////////////////////////////////////////////////

	extern DXGI_FORMAT GetFormat(ETextureFormat eFormat);

	class IOETexture_Platform : public IOETexture_PlatformBase
	{
	public:
		IOETexture_Platform(IOETexture &rTexture);
		~IOETexture_Platform();

	public:
		virtual bool Setup(IOETextureSource *pSource = nullptr) override final;
		virtual void Cleanup() override final;
		// bool CreateTexture(ID3D11Resource *&rpReturnResult, ID3D11Device
		// *pDevice);

	public:
		bool CreateView();
		void ReleaseView();

		bool CreateTexture(IOETextureSource *pSource = nullptr);
		bool CreateStagingTexture(IOETextureSource *pSource = nullptr);

		void ReleaseTexture();

	public:
		virtual bool
		MapBuffer(EBufferMapType eMapType, void *&rpData,
				  std::uint32_t uSubresourceID = 0) override final;
		virtual bool
		UnmapBuffer(std::uint32_t uSubresourceID = 0) override final;

		virtual void OnResize() override final;

		virtual void
		LoadFromMemory(std::uint8_t *pMemoryAddress, std::uint32_t uWidth,
					   std::uint32_t uHeight,
					   std::uint32_t uPixelByteWidth = 4) override final;
		virtual void CopyTexture(IOETexture &rhs) override final;

		virtual void SetDebugName(const std::string &szName) override final;

	public:
		ID3D11Resource *GetD3D11Texture()
		{
			return m_pTexture.get();
		}

		const ID3D11Resource *GetD3D11Texture() const
		{
			return m_pTexture.get();
		}

		TComSharedPtr<ID3D11Resource> GetD3D11TextureShared()
		{
			return m_pTexture;
		}

		const TComSharedPtr<ID3D11Resource> GetD3D11TextureShared() const
		{
			return m_pTexture;
		}

		ID3D11View *GetView()
		{
			return m_pView.get();
		}
		
		const ID3D11View *GetView() const
		{
			return m_pView.get();
		}

		TComSharedPtr<ID3D11View> GetViewShared()
		{
			return m_pView;
		}

		const TComSharedPtr<ID3D11View> GetViewShared() const
		{
			return m_pView;
		}

	private:
		bool CreateTextureInternal(TComSharedPtr<ID3D11Resource> &rpResource,
								   IOETextureSource *pSource = nullptr,
								   bool bStaging = false);

		bool GetDescription1D(D3D11_TEXTURE1D_DESC &rDesc,
							  std::uint32_t uNumMipMaps,
							  bool bStaging = false) const;
		bool GetDescription2D(D3D11_TEXTURE2D_DESC &rDesc,
							  std::uint32_t uNumMipMaps,
							  bool bStaging = false) const;
		bool GetDescription3D(D3D11_TEXTURE3D_DESC &rDesc,
							  std::uint32_t uNumMipMaps,
							  bool bStaging = false) const;
		bool GetDescriptionCube(D3D11_TEXTURE2D_DESC &rDesc,
								std::uint32_t uNumMipMaps,
								bool bStaging = false) const;
		bool GetDescriptionBuffer(D3D11_BUFFER_DESC &rDesc,
								  bool bStaging = false) const;

	private:
		TComSharedPtr<ID3D11Resource> m_pTexture;
		TComSharedPtr<ID3D11Resource> m_pStagingTexture;
		TComSharedPtr<ID3D11View> m_pView;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE