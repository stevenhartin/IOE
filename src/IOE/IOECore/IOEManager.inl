template <typename _T>
IOEManager<_T>::IOEManager(const std::string &szName)
	: IManagerBase()
{
	ms_szManagerName = szName;
}

template <typename _T>
IOEManager<_T>::~IOEManager()
{
	OnDestroy();
}

template <typename _T>
void IOEManager<_T>::CacheSingleton()
{
	if (nullptr == ms_pSingleton)
	{
		// Attempt to find and cache off reference to shared_ptr
		ms_pSingleton = IOE::Core::g_arrManagers.FindManager<_T>(ms_szManagerName).get();
		IOE_ASSERT_VARGS(
			ms_pSingleton != nullptr, "Failed to find manager '%ls'",
			::IOE::Core::Algorithm::ConvertNarrowToWide(ms_szManagerName)
				.c_str());
	}
}

template <typename _T>
_T *IOEManager<_T>::GetSingletonPtr()
{
	CacheSingleton();
	return ms_pSingleton;
}

template <typename _T>
void IOEManager<_T>::OnCreate()
{
}

template <typename _T>
void IOEManager<_T>::OnManagerPreInit()
{
}

template <typename _T>
void IOEManager<_T>::OnManagerInit()
{
}

template <typename _T>
void IOEManager<_T>::OnManagerPostInit()
{
}

template <typename _T>
void IOEManager<_T>::OnPreUpdate(IOE::Core::IOETimeDelta TimeDelta)
{
}

template <typename _T>
void IOEManager<_T>::OnUpdate(IOE::Core::IOETimeDelta TimeDelta)
{
}

template <typename _T>
void IOEManager<_T>::OnPostUpdate(IOE::Core::IOETimeDelta TimeDelta)
{
}

template <typename _T>
void IOEManager<_T>::OnPreRender(IOE::Renderer::IOERPI *pPtr)
{
}

template <typename _T>
void IOEManager<_T>::OnRender(IOE::Renderer::IOERPI *pPtr)
{
}

template <typename _T>
void IOEManager<_T>::OnPostRender(IOE::Renderer::IOERPI *pPtr)
{
}

template <typename _T>
void IOEManager<_T>::OnDestroy()
{
}