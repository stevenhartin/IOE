#pragma once

#include <vector>
#include <memory>
#include <utility>

#include <IOE/IOECore/IOEManager.h>
#include <IOE/IOEExceptions/IOEExceptionList.h>

using std::shared_ptr;

namespace IOE
{
namespace Renderer
{
	class IOERPI;
}
namespace Core
{
	class IOEManagerContainer
	{
	public:
		IOEManagerContainer();

	private:
		FORCEINLINE_DEBUGGABLE shared_ptr<IManagerBase>
		__InternalFindManager(const std::string &szManagerName)
		{
			for (auto &pManager : m_arrManagers)
			{
				if (szManagerName == pManager->GetName())
				{
					return pManager;
				}
			}
			THROW_IOE_BASE_EXCEPTION("Manager does not exist",
									 E_ERROR_INVALID_KEY);
		}

		FORCEINLINE_DEBUGGABLE const shared_ptr<IManagerBase>
		__InternalFindManager(const std::string &szManagerName) const
		{
			for (const auto &pManager : m_arrManagers)
			{
				if (szManagerName == pManager->GetName())
				{
					return pManager;
				}
			}
			return nullptr;
		}

	public:
		template <class _T>
		FORCEINLINE_DEBUGGABLE shared_ptr<_T>
		FindManager(const std::string &szManagerName)
		{
			shared_ptr<_T> pCastManager(std::static_pointer_cast<_T>(
				__InternalFindManager(szManagerName)));
			return pCastManager;
		}

		shared_ptr<IManagerBase> operator[](const std::string &szManagerName)
		{
			try
			{
				return __InternalFindManager(szManagerName);
			}
			catch (IOE::Exceptions::IOEBaseException &e)
			{
				throw e;
			}
		}

	public:
		template <class T, class... _TArgs>
		inline shared_ptr<T> CreateManager(_TArgs &&... Args)
		{
			shared_ptr<T> pManager(
				std::make_shared<T>(std::forward<_TArgs>(Args...)...));
			m_arrManagers.push_back(pManager);
			pManager->OnCreate();
			return pManager;
		}

	public:
		virtual void OnManagerPreInit();
		virtual void OnManagerInit();
		virtual void OnManagerPostInit();

		virtual void OnPreUpdate(IOE::Core::IOETimeDelta TimeDelta);
		virtual void OnUpdate(IOE::Core::IOETimeDelta TimeDelta);
		virtual void OnPostUpdate(IOE::Core::IOETimeDelta TimeDelta);

		virtual void OnPreRender();
		virtual void OnRender();
		virtual void OnPostRender();

	private:
		std::vector<shared_ptr<IManagerBase> > m_arrManagers;
	};

	extern IOEManagerContainer g_arrManagers;

} // namespace Core
} // namespace IOE