#pragma once

//////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <map>

#include <IOE/IOECore/IOEDefines.h>

//////////////////////////////////////////////////////////////////////////

using std::vector;
using std::string;
using std::map;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Input
{

	//////////////////////////////////////////////////////////////////////////

	enum class EInputDeviceType
	{
		Unknown,
		Keyboard,
		Mouse
	};

	//////////////////////////////////////////////////////////////////////////

	class IOEInputDevice
	{
	public:
		static const EInputDeviceType ms_DeviceType =
			EInputDeviceType::Unknown;

	public:
		IOEInputDevice(EInputDeviceType eDeviceType = ms_DeviceType)
			: m_bEnabled(true)
			, m_eDeviceType(eDeviceType)
		{
		}

		FORCEINLINE void Enable()
		{
			if (!m_bEnabled)
			{
				m_bEnabled = true;
				OnEnable();
			}
		}

		FORCEINLINE void Disable()
		{
			if (m_bEnabled)
			{
				m_bEnabled = false;
				OnDisable();
			}
		}
		FORCEINLINE bool IsEnabled() const
		{
			return m_bEnabled;
		}

	public:
		virtual void OnEnable()
		{
		}
		virtual void OnDisable()
		{
		}

		virtual void SetUp()
		{
		}
		virtual void Process()
		{
		}
		virtual void CleanUp()
		{
		}

		FORCEINLINE EInputDeviceType GetDeviceType()
		{
			return m_eDeviceType;
		}

	private:
		bool m_bEnabled;
		EInputDeviceType m_eDeviceType;
	};

	/////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE