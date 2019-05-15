#pragma once

//////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <map>
#include <memory>

#include <IOE/IOECore/IOEDefines.h>
#include <IOE/IOECore/IOEManager.h>

#include PLATFORM_INCLUDE(IOEInputManager_Platform.h)

//////////////////////////////////////////////////////////////////////////

using std::vector;
using std::string;
using std::map;
using std::shared_ptr;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Input
{

	//////////////////////////////////////////////////////////////////////////

	class IOEInputDevice;

	//////////////////////////////////////////////////////////////////////////

	class IOEInputManager : public IOE::Core::IOEManager<IOEInputManager>
	{
	public:
		IOEInputManager()
			: IOEManager("InputManager")
		{
		}

		template <typename T>
		FORCEINLINE_DEBUGGABLE void CreateDevice()
		{
			auto pDevice(std::make_shared<T>());
			m_arrDevices.push_back(pDevice);
			pDevice->SetUp();
		}

		virtual void OnManagerPreInit() override;
		virtual void OnPreUpdate(IOE::Core::IOETimeDelta TimeDelta) override;

		IOEInputManager_Platform &GetPlatform()
		{
			return m_tPlatform;
		}
		const IOEInputManager_Platform &GetPlatform() const
		{
			return m_tPlatform;
		}

		template <typename _T>
		FORCEINLINE_DEBUGGABLE shared_ptr<_T> FindDeviceShared()
		{
			for (auto pDevice : m_arrDevices)
			{
				if (pDevice->GetDeviceType() == _T::ms_DeviceType)
				{
					return std::static_pointer_cast<_T>(pDevice);
				}
			}
			return nullptr;
		}

		template <typename _T>
		FORCEINLINE_DEBUGGABLE _T *FindDevice()
		{
			for (auto pDevice : m_arrDevices)
			{
				if (pDevice->GetDeviceType() == _T::ms_DeviceType)
				{
					return static_cast<_T*>(pDevice.get());
				}
			}
			return nullptr;
		}

	private:
		IOEInputManager_Platform m_tPlatform;
		vector<shared_ptr<IOEInputDevice> > m_arrDevices;
	};

	/////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE