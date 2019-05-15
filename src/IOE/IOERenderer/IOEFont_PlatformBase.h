#pragma once

#include <string>

#include <IOE/IOECore/IOEDefines.h>

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOEFont;

	//////////////////////////////////////////////////////////////////////////

	abstract_class IOEFont_PlatformBase
	{
	public:
		IOEFont_PlatformBase(IOEFont * pFont)
			: m_pFont(pFont)
		{
		}

	public:
		virtual void Setup(class IOERPI * pRPI) = 0;

		virtual void SetFontName(const std::string &szName) = 0;

	protected:
		IOEFont *GetFont()
		{
			return m_pFont;
		}
		const IOEFont *GetFont() const
		{
			return m_pFont;
		}

	private:
		IOEFont *m_pFont;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE