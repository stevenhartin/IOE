//////////////////////////////////////////////////////////////////////////

template <typename TApplicationType>
IOE::Core::IOEEngine<TApplicationType>::IOEEngine()
{
}

//////////////////////////////////////////////////////////////////////////

template <typename TApplicationType>
std::int32_t IOE::Core::IOEEngine<TApplicationType>::EnginePreInit()
{
	if (m_pApplication == nullptr)
	{
		try
		{
			m_pApplication =
				IOE::Core::g_arrManagers.FindManager<TApplicationType>(
					"IOEApplication");
		}
		catch (IOE::Exceptions::IOEBaseException &)
		{
			return static_cast<std::int32_t>(
				IOE::Exceptions::EExceptionID::E_ERROR_MANAGER_NOT_CREATED);
		}
	}
	m_pApplication->CreateApplication();

	IOE::Core::g_arrManagers.CreateManager<IOE::Renderer::IOEModelManager>();

	return 0;
}

/////////////////////////////////////////////////////////////////////////

template <typename TApplicationType>
std::int32_t IOE::Core::IOEEngine<TApplicationType>::EngineInit()
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////

template <typename TApplicationType>
std::int32_t IOE::Core::IOEEngine<TApplicationType>::EnginePostInit()
{
	// Initialise all managers, we expect that they should all have been
	// created
	// by now.
	g_arrManagers.OnManagerPreInit();
	g_arrManagers.OnManagerInit();
	g_arrManagers.OnManagerPostInit();

	return 0;
}

//////////////////////////////////////////////////////////////////////////

template <typename TApplicationType>
std::int32_t IOE::Core::IOEEngine<TApplicationType>::EngineLoop()
{
	IOEHighResolutionTime tLastUpdated;
	double dDelta(0.0);

	double dTimeDelta	 = 0.0;
	std::uint32_t uFrames = 0;

	while (true)
	{
		IOEHighResolutionTime tCurrentTime;

		if (GetFrameTimeLimiter() > 0.0)
		{
			while ((dDelta = tCurrentTime - tLastUpdated) <
				   GetFrameTimeLimiter())
			{
				if (dDelta < GetFrameTimeLimiterStopSleep())
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
				tCurrentTime = IOEHighResolutionTime();
			}
		}

		if (dDelta > GetMinFrameTime())
		{
			dDelta = GetMinFrameTime();
		}

		dDelta *= 0.001;
		tLastUpdated = tCurrentTime;

		IOETimeDelta tDelta(static_cast<float>(dDelta));
		SetFrameDelta(tDelta);

		++uFrames;
		dTimeDelta += tDelta.fDelta;
		if (dTimeDelta >= 1.0)
		{
			std::cout << "FPS: " << (uFrames / dTimeDelta) << std::endl;
			uFrames	= 0;
			dTimeDelta = 0.0;
		}

		g_arrManagers.OnPreUpdate(tDelta);
		g_arrManagers.OnUpdate(tDelta);
		g_arrManagers.OnPostUpdate(tDelta);

		g_arrManagers.OnPreRender();
		g_arrManagers.OnRender();
		g_arrManagers.OnPostRender();

		if (IsRequestingExit())
		{
			break;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

template <typename TApplicationType>
std::int32_t IOE::Core::IOEEngine<TApplicationType>::EnginePreStop()
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////

template <typename TApplicationType>
std::int32_t IOE::Core::IOEEngine<TApplicationType>::EngineStop()
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////

template <typename TApplicationType>
std::int32_t IOE::Core::IOEEngine<TApplicationType>::EnginePostStop()
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////

template <typename TApplicationType>
FORCEINLINE_DEBUGGABLE std::wstring
IOE::Core::IOEEngine<TApplicationType>::GetBinPath() const
{
	const IOE::Application::IOECommandLine &rCommandLine(
		GetApplication()->GetCommandLine());
	std::wstring szPath(rCommandLine.GetExecutionDir());
	return szPath;
}

//////////////////////////////////////////////////////////////////////////

template <typename TApplicationType>
FORCEINLINE_DEBUGGABLE std::wstring
IOE::Core::IOEEngine<TApplicationType>::GetMediaPath() const
{
	std::wstring szRootDir(
		IOE::Core::Algorithm::GetBasePath(GetBinPath(), false));
	return szRootDir + IOEPlatformStatics_Platform::GetPathSplit() + L"Media";
}

//////////////////////////////////////////////////////////////////////////