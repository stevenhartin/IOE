#pragma once

#include <string>
#include <memory>

#include <IOE/IOECore/IOEHeaders.h>
#include <IOE/IOECore/IOEHighResolutionTime.h>

#define DECLARE_MANAGER_CHILD(class_, manager_)                               \
	class class_ : public ::IOE::##manager_## ::IOE##manager_##<class_>

using std::shared_ptr;

namespace IOE
{
namespace Renderer
{
	class IOERPI;
}
namespace Core
{

	abstract_class IManagerBase
	{
	public:
		IManagerBase()
		{
		}
		virtual ~IManagerBase(){};

		virtual void OnCreate() = 0;

		virtual void OnManagerPreInit() = 0;
		virtual void OnManagerInit() = 0;
		virtual void OnManagerPostInit() = 0;

		virtual void OnPreUpdate(IOE::Core::IOETimeDelta TimeDelta) = 0;
		virtual void OnUpdate(IOE::Core::IOETimeDelta TimeDelta) = 0;
		virtual void OnPostUpdate(IOE::Core::IOETimeDelta TimeDelta) = 0;

		virtual void OnPreRender(IOE::Renderer::IOERPI * pRPI) = 0;
		virtual void OnRender(IOE::Renderer::IOERPI * pRPI) = 0;
		virtual void OnPostRender(IOE::Renderer::IOERPI * pRPI) = 0;

		virtual void OnDestroy() = 0;

		virtual const std::string &GetName() const = 0;
	};

	template <typename _T>
	class IOEManager : public IManagerBase,
					   public std::enable_shared_from_this<_T>
	{
	public:
		IOEManager(const std::string &szName);
		virtual ~IOEManager();

		virtual void OnCreate();

		virtual void OnManagerPreInit();
		virtual void OnManagerInit();
		virtual void OnManagerPostInit();

		virtual void OnPreUpdate(IOE::Core::IOETimeDelta TimeDelta);
		virtual void OnUpdate(IOE::Core::IOETimeDelta TimeDelta);
		virtual void OnPostUpdate(IOE::Core::IOETimeDelta TimeDelta);

		// Executed before the device is cleared. Do any setup code here
		virtual void OnPreRender(IOE::Renderer::IOERPI *pRPI);

		// Executed just after the device has been cleared and is ready
		// to start executing any render commands
		virtual void OnRender(IOE::Renderer::IOERPI *pRPI);

		// Executed after the device has been presented. Do any cleanup
		// code here.
		virtual void OnPostRender(IOE::Renderer::IOERPI *pRPI);

		virtual void OnDestroy();

		static _T *GetSingletonPtr();

		inline virtual const std::string &GetName() const override
		{
			return ms_szManagerName;
		}

	private:
		static void CacheSingleton();

	private:
		static std::string ms_szManagerName;
		static _T *ms_pSingleton;
	};

#include "IOEManager.inl"

	template <typename _T>
	std::string IOEManager<_T>::ms_szManagerName = std::string("UnnamedManager");

	template <typename _T>
	_T *IOEManager<_T>::ms_pSingleton = nullptr;

} // namespace Core
} // namespace IOE