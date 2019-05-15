#pragma once

#include <windows.h>
#include <stdint.h>
#include "../IOEApplication_PlatformBase.h"
#include <IOE/IOECore/IOEEngine.h>

#undef CREATE_PLATFORM_ENTRY
#ifdef CONSOLE_APPLICATION

#define CREATE_PLATFORM_ENTRY(classname)                                      \
	std::int32_t wmain(std::int32_t argc, wchar_t **argv)                     \
	{                                                                         \
		IOE::Application::C_CommandLine cCmdLine = { argc, argv };            \
		IOE::Core::g_arrManagers.CreateManager<classname>(cCmdLine);          \
		IOE::Core::g_pEngine =                                                \
			std::make_unique<IOE::Core::IOEEngine<classname> >();             \
		return IOE::Core::g_pEngine->Run();                                   \
	}

#else

#define CREATE_PLATFORM_ENTRY(classname)                                      \
	int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPreviousInst,              \
					   LPSTR szArgv, int nCmdShow)                            \
	{                                                                         \
		IOE::Core::MakeEngine<classname>();                                   \
		IOE::Application::C_CommandLine cCmdLine;                             \
		cCmdLine.arrArgV = CommandLineToArgvW(                                \
			GetCommandLineW(), reinterpret_cast<int *>(&cCmdLine.nArgC));     \
		auto pManager =                                                       \
			IOE::Core::g_arrManagers.CreateManager<classname>(cCmdLine);      \
		pManager->CreateManagerDependencies();                                \
		pManager->GetPlatform().SetApplicationHandle(hInst);                  \
		return IOE::Core::g_pEngine->Run();                                   \
	}
#endif

namespace IOE
{
namespace Application
{
	class IOEApplication_Platform : public IApplication_PlatformBase
	{
	public:
		virtual void OnUpdate(IOE::Core::IOETimeDelta TimeDelta);

		virtual void CreateApplication(IOEWindowOptions tOptions) override;

		FORCEINLINE void SetApplicationHandle(HINSTANCE tHandle)
		{
			m_tApplicationHandle = tHandle;
		}
		FORCEINLINE HINSTANCE GetApplicationHandle() const
		{
			return m_tApplicationHandle;
		}

		FORCEINLINE HWND GetApplicationWindow() const
		{
			return m_tApplicationWindow;
		}

		virtual void OnResize(std::uint32_t uWidth,
							  std::uint32_t uHeight) final override;

		virtual void Log(const char *szMessage,
						 EVerbosity eVerbosity = EVerbosity::Log) override;

	public:
		virtual void SetWindowTitle(const std::string &szText) override;
		virtual void GetWindowSize(std::uint32_t &uWidth,
								   std::uint32_t &uHeight) override;
		virtual void SetWindowSize(std::uint32_t uWidth,
								   std::uint32_t uHeight) override;

		virtual bool HasFocus() const override final;
		virtual void GetWindowPos(std::uint32_t &uX,
								  std::uint32_t &uY) override final;
		virtual void SetWindowPos(std::uint32_t uX,
								  std::uint32_t uY) override final;

	private:
		static LRESULT CALLBACK WndProc(HWND hWindow, UINT uMsg,
										WPARAM uWParam, LPARAM uLParam);

	private:
		void CreateGUIWindow();
		void CreateConsole();

		HINSTANCE m_tApplicationHandle;
		HWND m_tApplicationWindow;
	};

} // namespace Application
} // namespace IOL