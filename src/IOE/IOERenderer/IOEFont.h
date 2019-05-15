#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <utility>

#include <IOE/IOECore/IOEDefines.h>
#include <IOE/IOECore/IOEManager.h>

#include <IOE/IOEMaths/Vector.h>
#include <IOE/IOERenderer/IOEBuffer.h>
#include <IOE/IOERenderer/IOEVertexFormat.h>
#include <IOE/IOERenderer/IOERPI.h>

#include RPI_INCLUDE(IOEFont_Platform.h)

using IOE::Maths::IOEVector;

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOETexture;
	class IOERPI;
	class IOEMaterial;

	//////////////////////////////////////////////////////////////////////////

	namespace EFontCreationFlags
	{
		enum Enum
		{
			Bold,
		};
	}

	//////////////////////////////////////////////////////////////////////////

	struct IOEFontStyle
	{
		IOEVector vColour;
		float fScaleX, fScalyY;

		IOEFontStyle(const IOEVector &vColour_, float fScaleX_,
					 float fScaleY_);
	};

	//////////////////////////////////////////////////////////////////////////

	struct IOEFontGlyph
	{
		float fSizeX, fSizeY;
		float fTexMinX, fTexMinY;
		float fTexMaxX, fTexMaxY;

		IOEFontGlyph()
			: fSizeX(0.0f)
			, fSizeY(0.0f)
			, fTexMinX(0.0f)
			, fTexMinY(0.0f)
			, fTexMaxX(0.0f)
			, fTexMaxY(0.0f)
		{
		}

		IOEFontGlyph(float fSizeX_, float fSizeY_, float fTexMinX_,
					 float fTexMinY_, float fTexMaxX_, float fTexMaxY_)
			: fSizeX(fSizeX_)
			, fSizeY(fSizeY_)
			, fTexMinX(fTexMinX_)
			, fTexMinY(fTexMinY_)
			, fTexMaxX(fTexMaxX_)
			, fTexMaxY(fTexMaxY_)
		{
		}

		IOEFontGlyph(IOEFontGlyph &&rhs)
			: fSizeX(std::move(rhs.fSizeX))
			, fSizeY(std::move(rhs.fSizeY))
			, fTexMinX(std::move(rhs.fTexMinX))
			, fTexMinY(std::move(rhs.fTexMinY))
			, fTexMaxX(std::move(rhs.fTexMaxX))
			, fTexMaxY(std::move(rhs.fTexMaxY))
		{
		}

		IOEFontGlyph(const IOEFontGlyph &rhs)
			: fSizeX(rhs.fSizeX)
			, fSizeY(rhs.fSizeY)
			, fTexMinX(rhs.fTexMinX)
			, fTexMinY(rhs.fTexMinY)
			, fTexMaxX(rhs.fTexMaxX)
			, fTexMaxY(rhs.fTexMaxY)
		{
		}

		IOEFontGlyph &operator=(IOEFontGlyph &&rhs)
		{
			fSizeX   = std::move(rhs.fSizeX);
			fSizeY   = std::move(rhs.fSizeY);
			fTexMinX = std::move(rhs.fTexMinX);
			fTexMinY = std::move(rhs.fTexMinY);
			fTexMaxX = std::move(rhs.fTexMaxX);
			fTexMaxY = std::move(rhs.fTexMaxY);
		}

		IOEFontGlyph &operator=(const IOEFontGlyph &rhs)
		{
			fSizeX   = rhs.fSizeX;
			fSizeY   = rhs.fSizeY;
			fTexMinX = rhs.fTexMinX;
			fTexMinY = rhs.fTexMinY;
			fTexMaxX = rhs.fTexMaxX;
			fTexMaxY = rhs.fTexMaxY;
		}
	};

	//////////////////////////////////////////////////////////////////////////

	class IOEFont
	{
	public:
		IOEFont();

		static std::shared_ptr<IOEFont> CreateFontFromName(
			const std::string &szFontName, std::uint32_t uHeight,
			EFontCreationFlags::Enum eFlags = (EFontCreationFlags::Enum)0);

		void Setup(IOERPI *pRPI);

		std::uint32_t GetHeight() const
		{
			return m_uHeight;
		}
		EFontCreationFlags::Enum GetCreationFlags() const
		{
			return m_eCreationFlag;
		}

		void EmplaceGlyph(std::uint32_t uGlyphIdx, float fSizeX_,
						  float fSizeY_, float fTexMinX_, float fTexMinY_,
						  float fTexMaxX_, float fTexMaxY_)
		{
			m_arrGlyphs[uGlyphIdx] = IOEFontGlyph(
				fSizeX_, fSizeY_, fTexMinX_, fTexMinY_, fTexMaxX_, fTexMaxY_);
		}

		void EmplaceTextureByte(std::uint32_t uTextureIdx,
								std::uint32_t uTextureByte)
		{
			m_arrTextureSource[uTextureIdx] = uTextureByte;
		}

		IOEFontGlyph &GetGlyph(std::uint32_t uGlyphIndex)
		{
			return m_arrGlyphs[uGlyphIndex];
		}
		const IOEFontGlyph &GetGlyph(std::uint32_t uGlyphIndex) const
		{
			return m_arrGlyphs[uGlyphIndex];
		}

		IOEFontGlyph *GetGlyphFromChar(char chCharacter)
		{
			return chCharacter >= 32 ? &m_arrGlyphs[chCharacter - 32]
									 : nullptr;
		}
		const IOEFontGlyph *GetGlyphFromChar(char chCharacter) const
		{
			return chCharacter >= 32 ? &m_arrGlyphs[chCharacter - 32]
									 : nullptr;
		}

		void SetTextureSize(float fWidth, float fHeight)
		{
			m_fTextureWidth  = fWidth;
			m_fTextureHeight = fHeight;
		}

		IOEBufferBase *GetIndexBuffer()
		{
			return &m_tIndexBuffer;
		}
		const IOEBufferBase *GetIndexBuffer() const
		{
			return &m_tIndexBuffer;
		}

		std::shared_ptr<IOETexture> GetTextureShared()
		{
			return m_pTexture;
		}

		const std::shared_ptr<IOETexture> GetTextureShared() const
		{
			return m_pTexture;
		}

		IOETexture *GetTexture()
		{
			return m_pTexture.get();
		}

		const IOETexture *GetTexture() const
		{
			return m_pTexture.get();
		}

	private:
		IOEFont_Platform m_tPlatform;

		std::shared_ptr<IOETexture> m_pTexture;

		IOEBuffer<std::uint32_t> m_tIndexBuffer;

		std::uint32_t m_uHeight;
		EFontCreationFlags::Enum m_eCreationFlag;

		std::array<IOEFontGlyph, 128> m_arrGlyphs;
		std::array<std::uint32_t, 4096 * 4096> m_arrTextureSource;

		float m_fTextureWidth;
		float m_fTextureHeight;
	};

	//////////////////////////////////////////////////////////////////////////

	class IOEText
	{
	public:
		static const std::uint32_t ms_uMaxNumChars = 128;

	public:
		IOEText(const std::string &szData, float fX, float fY,
				std::shared_ptr<IOEFont> pFont,
				const IOEVector &vColourBegin = Maths::ColourList::White,
				const IOEVector &vColourEnd = Maths::ColourList::White);

		void Setup(IOERPI *pRPI);

		void UpdateScale(float fScaleX, float fScaleY);
		void UpdateText(const std::string &szData);
		void UpdatePosition(float fX, float fY);
		void UpdateColour(const IOEVector &vColour);
		void UpdateColour(const IOEVector &vColourBegin,
						  const IOEVector &vColourEnd);

		void OnRender(IOE::Renderer::IOERPI *pPtr);

		bool Map(void *&rpData);
		bool Unmap();

		float GetRenderWidth();
		float GetRenderHeight();

		IOEFont *GetFont()
		{
			return m_pFont.get();
		}
		const IOEFont *GetFont() const
		{
			return m_pFont.get();
		}

	private:
		void UpdateData();

	private:
		std::shared_ptr<IOEFont> m_pFont;
		std::string m_szData;
		float m_fX, m_fY;
		float m_fScaleX, m_fScaleY;
		IOEVector m_vColourBegin;
		IOEVector m_vColourEnd;

		IOEBuffer<FontVertex> m_tVertexBuffer;

		bool m_bSetup;
	};

	//////////////////////////////////////////////////////////////////////////

	class IOETextManager : public IOE::Core::IOEManager<IOETextManager>
	{
	public:
		IOETextManager();

		std::shared_ptr<IOEText> CreateText(
			const std::string &rszData, float fX, float fY,
			std::shared_ptr<IOEFont> pFont,
			const IOEVector &vColourBegin = IOE::Maths::ColourList::White,
			const IOEVector &vColourEnd = IOE::Maths::ColourList::White);

		FORCEINLINE void SetVisible(bool bVisible)
		{
			m_bVisible = bVisible;
		}
		FORCEINLINE bool GetVisible() const
		{
			return m_bVisible;
		}

	public:
		virtual void OnManagerPostInit() override final;

		virtual void OnRender(IOE::Renderer::IOERPI *pPtr) override final;

	private:
		std::shared_ptr<IOEMaterial> m_pFontShader;

		std::vector<std::shared_ptr<IOEText> > m_arrTextInstances;

		bool m_bVisible;

		IOEBlendState tAlphaBlending;
		IOEDepthState tDepthDisable;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE