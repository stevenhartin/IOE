#pragma once

#include <cstdint>

#include <IOE/IOECore/IOEDefines.h>
#include <IOE/IOECore/IOEHighResolutionTime.h>

namespace IOE
{
namespace Application
{
	struct IOEWindowOptions
	{
		std::uint8_t bCreateWindow : 1;
		std::uint8_t bCreateConsole : 1;

		IOEWindowOptions(const IOEWindowOptions &) = default;
		IOEWindowOptions(IOEWindowOptions &&) = default;
		IOEWindowOptions &operator=(const IOEWindowOptions &) = default;
		IOEWindowOptions &operator=(IOEWindowOptions &&) = default;

		IOEWindowOptions()
			: bCreateWindow(true)
			, bCreateConsole(true)
		{
		}
	};

	enum class EVerbosity
	{
		Verbose,
		Log,
		Warn,
		Error
	};

	interface_class IApplication_PlatformBase
	{
	public:
		virtual void OnUpdate(IOE::Core::IOETimeDelta TimeDelta) = 0;

		virtual void CreateApplication(IOEWindowOptions tOptions) = 0;

	public:
		virtual void SetWindowTitle(const std::string &szText) = 0;
		virtual void GetWindowSize(std::uint32_t & uWidth,
								   std::uint32_t & uHeight) = 0;
		virtual void SetWindowSize(std::uint32_t uWidth,
								   std::uint32_t uHeight) = 0;

		virtual void OnResize(std::uint32_t uWidth, std::uint32_t uHeight)
		{
			m_uWidth  = uWidth;
			m_uHeight = uHeight;
		}

		virtual void Log(const char *szMessage,
						 EVerbosity eVerbosity = EVerbosity::Log) = 0;

		FORCEINLINE std::uint32_t GetWidth() const
		{
			return m_uWidth;
		}
		FORCEINLINE std::uint32_t GetHeight() const
		{
			return m_uHeight;
		}

		virtual bool HasFocus() const = 0;
		virtual void GetWindowPos(std::uint32_t & uX, std::uint32_t & uY) = 0;
		virtual void SetWindowPos(std::uint32_t uX, std::uint32_t uY) = 0;

	private:
		std::uint32_t m_uWidth;
		std::uint32_t m_uHeight;
	};

} // namespace Application
} // namespace IOL