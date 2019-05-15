#include "IOEManagerContainer.h"

#include <IOE/IOERenderer/IOERPI.h>

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Core
{

	//////////////////////////////////////////////////////////////////////////

	IOEManagerContainer g_arrManagers;

	//////////////////////////////////////////////////////////////////////////

	IOEManagerContainer::IOEManagerContainer()
	{
		m_arrManagers.reserve(16);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEManagerContainer::OnManagerPreInit()
	{
		for (auto &pManager : m_arrManagers)
		{
			pManager->OnManagerPreInit();
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEManagerContainer::OnManagerInit()
	{
		for (auto &pManager : m_arrManagers)
		{
			pManager->OnManagerInit();
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEManagerContainer::OnManagerPostInit()
	{
		for (auto &pManager : m_arrManagers)
		{
			pManager->OnManagerPostInit();
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEManagerContainer::OnPreUpdate(IOE::Core::IOETimeDelta TimeDelta)
	{
		for (auto &pManager : m_arrManagers)
		{
			pManager->OnPreUpdate(TimeDelta);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEManagerContainer::OnUpdate(IOE::Core::IOETimeDelta TimeDelta)
	{
		for (auto &pManager : m_arrManagers)
		{
			pManager->OnUpdate(TimeDelta);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEManagerContainer::OnPostUpdate(IOE::Core::IOETimeDelta TimeDelta)
	{
		for (auto &pManager : m_arrManagers)
		{
			pManager->OnPostUpdate(TimeDelta);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEManagerContainer::OnPreRender()
	{
		IOE::Renderer::IOERPI *pPtr = IOE::Renderer::IOERPI::GetSingletonPtr();
		pPtr->OnPreRender(pPtr);
		for (auto &rIt = m_arrManagers.begin(); rIt != m_arrManagers.end();
			 ++rIt)
		{
			IOE::Core::IManagerBase *pManager = (*rIt).get();
			if (pManager != pPtr)
			{
				pManager->OnPreRender(pPtr);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEManagerContainer::OnRender()
	{
		IOE::Renderer::IOERPI *pPtr = IOE::Renderer::IOERPI::GetSingletonPtr();
		pPtr->OnRender(pPtr);
		for (auto &rIt = m_arrManagers.begin(); rIt != m_arrManagers.end();
			 ++rIt)
		{
			IOE::Core::IManagerBase *pManager = (*rIt).get();
			if (pManager != pPtr)
			{
				pManager->OnRender(pPtr);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEManagerContainer::OnPostRender()
	{
		IOE::Renderer::IOERPI *pPtr = IOE::Renderer::IOERPI::GetSingletonPtr();
		pPtr->OnPostRender(pPtr);
		for (auto &rIt = m_arrManagers.begin(); rIt != m_arrManagers.end();
			 ++rIt)
		{
			IOE::Core::IManagerBase *pManager = (*rIt).get();
			if (pManager != pPtr)
			{
				pManager->OnPostRender(pPtr);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Core
} // namespace IOE