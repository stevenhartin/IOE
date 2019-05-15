#pragma once

#include <IOE/IOECore/IOEHeaders.h>
#include <IOE/IOEExceptions/IOEExceptionList.h>
#include <IOE/IOECore/IOEManager.h>
#include <IOE/IOECore/IOEHighResolutionTime.h>
#include <IOE/IOEApplication/IOECommandLine.h>

#define CREATE_PLATFORM_ENTRY(classname) ((void)0)

#include PLATFORM_INCLUDE(IOEApplication_Platform.h)

namespace IOE
{
namespace Application
{
	extern const std::uint32_t gsDefaultResX;
	extern const std::uint32_t gsDefaultResY;

	template <typename _T>
	inline _T GetDefaultResX()
	{
		auto pBase(IOE::Core::g_pEngine->GetApplication());
		std::uint32_t uValue(gsDefaultResX);
		if (pBase->GetCommandLine().HasArgument(L"resx"))
		{
			uValue = pBase->GetCommandLine().GetArgumentValue<std::uint32_t>(
				L"resx", 0);
		}
		return static_cast<_T>(uValue);
	}

	template <typename _T>
	inline _T GetDefaultResY()
	{
		auto pBase(IOE::Core::g_pEngine->GetApplication());
		std::uint32_t uValue(gsDefaultResY);
		if (pBase->GetCommandLine().HasArgument(L"resy"))
		{
			uValue = pBase->GetCommandLine().GetArgumentValue<std::uint32_t>(
				L"resy", 0);
		}
		return static_cast<_T>(uValue);
	}

	interface_class IOEApplicationBase
	{
	public:
		virtual const IOECommandLine &GetCommandLine() const = 0;
		virtual void SetWindowTitle(const std::string &szText) = 0;
		virtual void SetWindowSize(std::uint32_t uWidth,
								   std::uint32_t uHeight) = 0;
		virtual void GetWindowSize(std::uint32_t & uWidth,
								   std::uint32_t & uHeight) = 0;
		virtual IOEApplication_Platform &GetPlatform() = 0;
		virtual const IOEApplication_Platform &GetPlatform() const = 0;

		virtual bool HasFocus() const = 0;
		virtual void GetWindowPos(std::uint32_t & uX, std::uint32_t & uY) = 0;
		virtual void SetWindowPos(std::uint32_t uX, std::uint32_t uY) = 0;

		virtual void Log(const char *szMessage,
						 EVerbosity eVerbosity = EVerbosity::Log) = 0;

		void Logf(const char *szMessage, ...)
		{
			char buffer[1024];
			va_list tArgs;
			va_start(tArgs, szMessage);
			vsprintf_s(buffer, 1024, szMessage, tArgs);
			Log(buffer, EVerbosity::Log);
			va_end(tArgs);
		}

		void Warnf(const char *szMessage, ...)
		{
			char buffer[1024];
			va_list tArgs;
			va_start(tArgs, szMessage);
			vsprintf_s(buffer, 1024, szMessage, tArgs);
			Log(buffer, EVerbosity::Warn);
			va_end(tArgs);
		}

		void Verbosef(const char *szMessage, ...)
		{
			char buffer[1024];
			va_list tArgs;
			va_start(tArgs, szMessage);
			vsprintf_s(buffer, 1024, szMessage, tArgs);
			Log(buffer, EVerbosity::Verbose);
			va_end(tArgs);
		}

		void Errorf(const char *szMessage, ...)
		{
			char buffer[1024];
			va_list tArgs;
			va_start(tArgs, szMessage);
			vsprintf_s(buffer, 1024, szMessage, tArgs);
			Log(buffer, EVerbosity::Error);
			va_end(tArgs);
		}

	public:
		virtual void OnResize(std::uint32_t uwidth, std::uint32_t uHeight) = 0;
	};

	template <class TApplication>
	class IOEApplication : public IOEApplicationBase,
						   public IOE::Core::IOEManager<TApplication>
	{
	public:
		IOEApplication(const C_CommandLine &rCommandLine)
			: IOEManager("IOEApplication")
		{
			vector<wstring> szArgs;
			szArgs.reserve(rCommandLine.nArgC);
			std::int32_t nIdx(0);
			std::generate_n(std::back_inserter(szArgs), rCommandLine.nArgC - 1,
							[&rCommandLine, &nIdx]() -> wstring {
								return wstring(rCommandLine.arrArgV[++nIdx]);
							});
			m_tCommandLine.ParseCommandLine(rCommandLine.arrArgV[0], szArgs);
		}

		FORCEINLINE virtual void OnUpdate(IOE::Core::IOETimeDelta TimeDelta)
		{
			m_tPlatform.OnUpdate(TimeDelta);
		}

		FORCEINLINE virtual void OnResize(std::uint32_t uWidth,
										  std::uint32_t uHeight)
		{
			m_tPlatform.OnResize(uWidth, uHeight);
			m_tPlatform.GetWindowSize(m_uWidth, m_uHeight);
		}

		FORCEINLINE virtual void CreateApplication()
		{
			m_tPlatform.CreateApplication(GetWindowOptions());
		}

		FORCEINLINE virtual IOEWindowOptions GetWindowOptions()
		{
			IOEWindowOptions tWindowOptions;
			return tWindowOptions;
		}

		FORCEINLINE virtual const IOECommandLine &
		GetCommandLine() const override final
		{
			return m_tCommandLine;
		}

		FORCEINLINE virtual IOEApplication_Platform &
		GetPlatform() override final
		{
			return m_tPlatform;
		}

		FORCEINLINE virtual const IOEApplication_Platform &
		GetPlatform() const override final
		{
			return m_tPlatform;
		}

		virtual void Log(const char *szMessage,
						 EVerbosity eVerbosity = EVerbosity::Log)
		{
			m_tPlatform.Log(szMessage, eVerbosity);
		}

		virtual void CreateManagerDependencies()
		{
		}

	public:
		FORCEINLINE virtual void
		SetWindowTitle(const std::string &szText) override final
		{
			m_tPlatform.SetWindowTitle(szText);
		}

		FORCEINLINE virtual void
		SetWindowSize(std::uint32_t uWidth,
					  std::uint32_t uHeight) override final
		{
			m_tPlatform.SetWindowSize(uWidth, uHeight);
		}

		FORCEINLINE virtual void
		GetWindowSize(std::uint32_t &uWidth,
					  std::uint32_t &uHeight) override final
		{
			uWidth  = m_uWidth;
			uHeight = m_uHeight;
		}

		virtual bool HasFocus() const override final
		{
			return m_tPlatform.HasFocus();
		}

		virtual void GetWindowPos(std::uint32_t &uX,
								  std::uint32_t &uY) override final
		{
			m_tPlatform.GetWindowPos(uX, uY);
		}

		virtual void SetWindowPos(std::uint32_t uX,
								  std::uint32_t uY) override final
		{
			m_tPlatform.SetWindowPos(uX, uY);
		}

	private:
		IOEApplication_Platform m_tPlatform;
		IOECommandLine m_tCommandLine;

		std::uint32_t m_uWidth;
		std::uint32_t m_uHeight;
	};

} // namespace Application
} // namespace IOL