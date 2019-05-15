//////////////////////////////////////////////////////////////////////////

#include <io.h>
#include <fcntl.h>
#include <fstream>
#include <cstdint>

#include <IOE/IOEApplication/IOEApplication.h>
#include "IOEApplication_Platform.h"

#include <IOE/IOECore/IOEManagerContainer.h>
#include <IOE/IOEExceptions/IOEExceptionList.h>
#include <IOE/IOERenderer/IOERPI.h>

//////////////////////////////////////////////////////////////////////////

using std::uint32_t;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Application
{

	//////////////////////////////////////////////////////////////////////////

	void IOE::Application::IOEApplication_Platform::CreateApplication(
		IOEWindowOptions tOptions)
	{
		if (!(tOptions.bCreateWindow || tOptions.bCreateConsole))
		{
			THROW_IOE_BASE_EXCEPTION("Must create window or console",
									 E_ERROR_INVALID_OPTIONS);
		}
		if (tOptions.bCreateConsole)
		{
			CreateConsole();
		}
		if (tOptions.bCreateWindow)
		{
			CreateGUIWindow();
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEApplication_Platform::CreateGUIWindow()
	{
		// this struct holds information for the window class
		WNDCLASSEX wc;

		// clear out the window class for use
		ZeroMemory(&wc, sizeof(WNDCLASSEX));

		// fill in the struct with the needed information
		wc.cbSize		 = sizeof(WNDCLASSEX);
		wc.style		 = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc   = &IOEApplication_Platform::WndProc;
		wc.hInstance	 = GetApplicationHandle();
		wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wc.lpszClassName = "IOE";

		// register the window class
		RegisterClassEx(&wc);

		// create the window and use the result as the handle
		m_tApplicationWindow =
			CreateWindowEx(NULL,
						   "IOE", // name of the window class
						   "Independent Open Engine", // title of the window
						   WS_OVERLAPPEDWINDOW,		  // window style
						   CW_USEDEFAULT, // x-position of the window
						   CW_USEDEFAULT, // y-position of the window
						   GetDefaultResX<uint32_t>(), // width of the window
						   GetDefaultResY<uint32_t>(), // height of the window
						   NULL, // we have no parent window, NULL
						   NULL, // we aren't using menus, NULL
						   GetApplicationHandle(), // application handle
						   NULL); // used with multiple windows, NULL

		// display the window on the screen
		ShowWindow(GetApplicationWindow(), SW_SHOWDEFAULT);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEApplication_Platform::CreateConsole()
	{
		/*int hConHandle;
		intptr_t lStdHandle;
		CONSOLE_SCREEN_BUFFER_INFO coninfo;*/
		FILE *fp;

		// allocate a console for this app
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		freopen_s(&fp, "CON", "w", stdout);
		freopen_s(&fp, "CON", "w", stderr);
		freopen_s(&fp, "CON", "r", stdin);

		// set the screen buffer to be big enough to let us scroll text
		/*GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
		&coninfo);
		coninfo.dwSize.Y = 500;
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
		coninfo.dwSize);

		// redirect unbuffered STDOUT to the console
		lStdHandle = (intptr_t)GetStdHandle(STD_OUTPUT_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

		fp = _fdopen(hConHandle, "w");

		*stdout = *fp;

		setvbuf(stdout, NULL, _IONBF, 0);

		// redirect unbuffered STDIN to the console

		lStdHandle = (intptr_t)GetStdHandle(STD_INPUT_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

		fp = _fdopen(hConHandle, "r");
		*stdin = *fp;
		setvbuf(stdin, NULL, _IONBF, 0);

		// redirect unbuffered STDERR to the console
		lStdHandle = (intptr_t)GetStdHandle(STD_ERROR_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

		fp = _fdopen(hConHandle, "w");

		*stderr = *fp;

		setvbuf(stderr, NULL, _IONBF, 0);

		// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
		// point to console as well
		std::ios::sync_with_stdio();*/
	}

	//////////////////////////////////////////////////////////////////////////

	LRESULT CALLBACK IOEApplication_Platform::WndProc(HWND hWindow, UINT uMsg,
													  WPARAM uWParam,
													  LPARAM uLParam)
	{
		switch (uMsg)
		{
		case WM_DESTROY:
		{
			// close the application entirely
			PostQuitMessage(0);
			IOE::Core::g_pEngine->RequestExit();
			return 0;
		}
		case WM_SIZE:
		{
			RECT tRect;
			if (::GetClientRect(hWindow, &tRect))
			{
				IOE::Core::g_pEngine->GetApplication()->OnResize(
					tRect.right - tRect.left, tRect.bottom - tRect.top);
				IOE::Renderer::IOERPI::GetSingletonPtr()->Resize(
					static_cast<float>(tRect.right - tRect.left),
					static_cast<float>(tRect.bottom - tRect.top));
			}
			break;
		}
		default:
			break;
		}

		// Handle any messages the switch statement didn't
		return DefWindowProc(hWindow, uMsg, uWParam, uLParam);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEApplication_Platform::OnUpdate(IOE::Core::IOETimeDelta TimeDelta)
	{
		// this struct holds Windows event messages
		MSG msg;

		// Check to see if any messages are waiting in the queue
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// Translate the message and dispatch it to WindowProc()
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If the message is WM_QUIT, exit the while loop
		if (msg.message == WM_QUIT)
		{
			IOE::Core::g_pEngine->RequestExit();
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEApplication_Platform::OnResize(std::uint32_t uWidth,
										   std::uint32_t uHeight)
	{
		IApplication_PlatformBase::OnResize(uWidth, uHeight);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEApplication_Platform::SetWindowTitle(const std::string &szText)
	{
		::SetWindowText(GetApplicationWindow(), szText.c_str());
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEApplication_Platform::GetWindowSize(std::uint32_t &uWidth,
												std::uint32_t &uHeight)
	{
		RECT tRect;
		uWidth  = 0;
		uHeight = 0;
		if (::GetClientRect(GetApplicationWindow(), &tRect))
		{
			uWidth  = tRect.right - tRect.left;
			uHeight = tRect.bottom - tRect.top;
			;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEApplication_Platform::SetWindowSize(std::uint32_t uWidth,
												std::uint32_t uHeight)
	{
		RECT tRect;
		if (::GetClientRect(GetApplicationWindow(), &tRect))
		{
			::SetWindowPos(GetApplicationWindow(), nullptr, tRect.top,
						   tRect.left, uWidth, uHeight,
						   SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void
	IOEApplication_Platform::Log(const char *szMessage,
								 EVerbosity eVerbosity /*=EVerbosity::Log*/)
	{
		printf(szMessage);
		OutputDebugStringA(szMessage);
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEApplication_Platform::HasFocus() const
	{
		HWND pWindow(::GetFocus());
		HWND pForeground(::GetForegroundWindow());
		return (pWindow == m_tApplicationWindow) &&
			(pForeground == m_tApplicationWindow);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEApplication_Platform::GetWindowPos(std::uint32_t &uX,
											   std::uint32_t &uY)
	{
		uX = 0;
		uY = 0;
		RECT tRect;
		if (GetClientRect(m_tApplicationWindow, &tRect))
		{
			uX = static_cast<uint32_t>(tRect.left);
			uY = static_cast<uint32_t>(tRect.top);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEApplication_Platform::SetWindowPos(std::uint32_t uX,
											   std::uint32_t uY)
	{
		::SetWindowPos(m_tApplicationWindow, nullptr, static_cast<int32_t>(uX),
					   static_cast<int32_t>(uY), 0, 0,
					   SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	//////////////////////////////////////////////////////////////////////////
}
}

//////////////////////////////////////////////////////////////////////////