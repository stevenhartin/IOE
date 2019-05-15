#include <d3d12.h>

#include "IOEFont_Platform.h"
#include "../IOEFont.h"
#include "../IOERPI.h"

//////////////////////////////////////////////////////////////////////////

static HFONT
CreateWinFont(HDC hDC, const std::string &szFontName, std::uint32_t uHeight,
			  IOE::Renderer::EFontCreationFlags::Enum eCreationFlags)
{
	LOGFONT tLogFont;

	tLogFont.lfHeight = -MulDiv(uHeight, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	tLogFont.lfWidth	   = 0;
	tLogFont.lfEscapement  = 0;
	tLogFont.lfOrientation = 0;
	tLogFont.lfWeight =
		eCreationFlags & IOE::Renderer::EFontCreationFlags::Bold ? FW_BOLD
																 : FW_NORMAL;
	tLogFont.lfItalic		  = FALSE;
	tLogFont.lfUnderline	  = FALSE;
	tLogFont.lfStrikeOut	  = FALSE;
	tLogFont.lfCharSet		  = DEFAULT_CHARSET;
	tLogFont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
	tLogFont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
	tLogFont.lfQuality		  = ANTIALIASED_QUALITY;
	tLogFont.lfPitchAndFamily = VARIABLE_PITCH;
	strncpy_s(tLogFont.lfFaceName, sizeof tLogFont.lfFaceName,
			  szFontName.c_str(), _TRUNCATE);

	HFONT hFont = CreateFontIndirect(&tLogFont);
	return hFont;
}

//////////////////////////////////////////////////////////////////////////

static bool GenerateGlyphs(HDC hDC, int nWidth, int nHeight,
						   IOE::Renderer::IOEFont *pFont)
{
	SIZE tGlyphSize;

	char chCharacter = 'x';
	GetTextExtentPoint32(hDC, &chCharacter, 1, &tGlyphSize);

	int nGlyphSpacing = int(ceilf(tGlyphSize.cy * .3f));

	int nGlyphWidth  = 0;
	int nGlyphHeight = 0;
	LONG uMaxHeight  = 0;

	for (int chCharacter = 32; chCharacter < 127; ++chCharacter)
	{
		char chGlyphCharacterCode = (char)chCharacter;
		if (!GetTextExtentPoint32(hDC, &chGlyphCharacterCode, 1, &tGlyphSize))
		{
			tGlyphSize.cx = 0;
			tGlyphSize.cy = 0;
		}

		if (nGlyphWidth + tGlyphSize.cx + nGlyphSpacing > nWidth)
		{
			nGlyphWidth = 0;
			nGlyphHeight += uMaxHeight + 1;

			uMaxHeight = 0;
		}

		if (nGlyphHeight + tGlyphSize.cy > nHeight)
		{
			if (pFont)
			{
				pFont->SetTextureSize((float)nWidth, (float)nHeight);
			}
			return false;
		}

		if (tGlyphSize.cy > uMaxHeight)
		{
			uMaxHeight = tGlyphSize.cy;
		}

		if (pFont != nullptr)
		{
			ExtTextOut(hDC, nGlyphWidth, nGlyphHeight, ETO_OPAQUE, nullptr,
					   &chGlyphCharacterCode, 1, nullptr);

			IOE::Renderer::IOEFontGlyph &rGlyph(
				pFont->GetGlyph(chCharacter - 32));

			rGlyph.fTexMinX = nGlyphWidth / float(nWidth);
			rGlyph.fTexMinY = nGlyphHeight / float(nHeight);

			rGlyph.fTexMaxX = (nGlyphWidth + tGlyphSize.cx) / float(nWidth);
			rGlyph.fTexMaxY = (nGlyphHeight + tGlyphSize.cy) / float(nHeight);

			rGlyph.fSizeX = float(tGlyphSize.cx);
			rGlyph.fSizeY = float(tGlyphSize.cy);
		}
		nGlyphWidth += tGlyphSize.cx + nGlyphSpacing;
	}

	if (pFont)
	{
		pFont->SetTextureSize((float)nWidth, (float)nHeight);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	IOEFont_Platform::IOEFont_Platform(IOEFont *pFont)
		: IOEFont_PlatformBase(pFont)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEFont_Platform::Setup(class IOERPI *pRPI)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEFont_Platform::SetFontName(const std::string &szName)
	{
		HDC hDC = CreateCompatibleDC(NULL);

		SetMapMode(hDC, MM_TEXT);
		SaveDC(hDC);

		HFONT hFont		= NULL;
		HBITMAP hBitmap = NULL;

		hFont = CreateWinFont(hDC, szName, GetFont()->GetHeight(),
							  GetFont()->GetCreationFlags());

		if (hFont)
		{
			SelectObject(hDC, hFont);

			int nRequiredTextureWidth = 128, nRequiredTextureHeight = 128;

			// Find the best texture size to use to fit all glyphs onto the
			// sheet at the given size.
			// Make sure we only go up to 4k, don't want to go crazy!
			while (!GenerateGlyphs(hDC, nRequiredTextureWidth,
								   nRequiredTextureHeight, nullptr) &&
				   (nRequiredTextureWidth != 4096 &&
					nRequiredTextureHeight != 4096))
			{
				nRequiredTextureWidth *= 2;
				nRequiredTextureHeight *= 2;
			}
			if (!(nRequiredTextureWidth == 4096 ||
				  nRequiredTextureHeight == 4096))
			{
				BITMAPINFO tBitmapInfo;
				memset(&tBitmapInfo, 0, sizeof tBitmapInfo);

				tBitmapInfo.bmiHeader.biSize  = sizeof tBitmapInfo.bmiHeader;
				tBitmapInfo.bmiHeader.biWidth = nRequiredTextureWidth;
				tBitmapInfo.bmiHeader.biHeight =
					-nRequiredTextureHeight; //-ve = top down
				tBitmapInfo.bmiHeader.biPlanes		= 1;
				tBitmapInfo.bmiHeader.biCompression = BI_RGB;
				tBitmapInfo.bmiHeader.biBitCount	= 32;

				DWORD *pByteData;

				hBitmap = CreateDIBSection(hDC, &tBitmapInfo, DIB_RGB_COLORS,
										   (void **)&pByteData, nullptr, 0);
				if (hBitmap)
				{
					SelectObject(hDC, hBitmap);

					SetTextColor(hDC, RGB(255, 255, 255));
					SetBkColor(hDC, RGB(0, 0, 0));
					SetTextAlign(hDC, TA_TOP);

					GenerateGlyphs(hDC, nRequiredTextureWidth,
								   nRequiredTextureHeight, GetFont());

					GdiFlush();

					for (int i = 0;
						 i < nRequiredTextureWidth * nRequiredTextureHeight;
						 ++i)
					{
						GetFont()->EmplaceTextureByte(
							i, ((pByteData[i] & 0xFF) << 24) | 0x00FFFFFF);
					}
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE

//////////////////////////////////////////////////////////////////////////