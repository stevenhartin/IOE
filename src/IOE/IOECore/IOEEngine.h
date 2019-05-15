#pragma once

#include <cstdint>
#include <memory>
#include <thread>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "IOEDefines.h"
#include "IOEHighResolutionTime.h"
#include PLATFORM_INCLUDE(IOEPlatformStatics_Platform.h)

namespace IOE
{
namespace Application
{
	interface_class IOEApplicationBase;
} // namesapce Application
namespace Core
{

	//////////////////////////////////////////////////////////////////////////

	enum class EEnginePhase : std::int32_t
	{
		PreInit,
		Init,
		PostInit,
		Loop,
		PreStop,
		Stop,
		PostStop
	};

	//////////////////////////////////////////////////////////////////////////

	class IOEEngineBase
	{
	public:
		IOEEngineBase();
		std::int32_t Run();

		FORCEINLINE EEnginePhase GetEnginePhase() const
		{
			return m_ePhase;
		}

		void RequestExit();

		virtual std::wstring GetBinPath() const = 0;
		virtual std::wstring GetMediaPath() const = 0;

		virtual IOETimeDelta GetFrameDelta() const
		{
			return m_dFrameDelta;
		}

	protected:
		virtual std::int32_t EnginePreInit() = 0;
		virtual std::int32_t EngineInit() = 0;
		virtual std::int32_t EnginePostInit() = 0;
		virtual std::int32_t EngineLoop() = 0;
		virtual std::int32_t EnginePreStop() = 0;
		virtual std::int32_t EngineStop() = 0;
		virtual std::int32_t EnginePostStop() = 0;

	public:
		virtual IOE::Application::IOEApplicationBase* GetApplication() = 0;
		virtual const IOE::Application::IOEApplicationBase*
		GetApplication() const = 0;
		virtual std::shared_ptr<IOE::Application::IOEApplicationBase>
		GetApplicationShared() = 0;
		virtual std::shared_ptr<const IOE::Application::IOEApplicationBase>
		GetApplicationShared() const = 0;

		FORCEINLINE void SetMaxFPS(float fValue)
		{
			if (fValue > 0.0f)
			{
				m_dFrameTimeLimiter = 1000.0 / static_cast<double>(fValue);
				m_dFrameTimeLimiterStopSleep = m_dFrameTimeLimiter * 0.67;
			}
			else
			{
				m_dFrameTimeLimiter = std::numeric_limits<double>::infinity();
				m_dFrameTimeLimiterStopSleep =
					std::numeric_limits<double>::infinity();
			}
		}
		FORCEINLINE void SetMinFPS(float fValue)
		{
			m_dMinFrameTime = 1000.0 / static_cast<double>(fValue);
		}

	public:
		std::int32_t IsRequestingExit() const
		{
			return m_uRequestingExit;
		}

		FORCEINLINE double GetFrameTimeLimiter() const
		{
			return m_dFrameTimeLimiter;
		}
		FORCEINLINE double GetFrameTimeLimiterStopSleep() const
		{
			return m_dFrameTimeLimiterStopSleep;
		}
		FORCEINLINE double GetMinFrameTime() const
		{
			return m_dMinFrameTime;
		}

	protected:
		FORCEINLINE void SetFrameDelta(IOETimeDelta dDelta)
		{
			m_dFrameDelta = dDelta;
		}

	private:
		FORCEINLINE void SetEnginePhase(EEnginePhase ePhase)
		{
			m_ePhase = ePhase;
		}
		EEnginePhase m_ePhase;

		std::int32_t m_uRequestingExit;

		// The maximum amount of ms the application is limited to. E.g.
		// 1000.0 / 60.0 = 60FPS cap
		double m_dFrameTimeLimiter;

		// Because sleep is an inaccurate command, we have to stop calling
		// sleep
		// when we're so far away, otherwise we're pretty likely to eat into
		// our
		// actual frame time and not get an accurate frame time limiter.
		double m_dFrameTimeLimiterStopSleep;

		// The minimum time a frame can be. This stops crazy small physics
		// values
		// being passed everywhere. All this will do is make the simulation of
		// the
		// game smaller than realistic values, so the game will begin to run
		// slower
		// but be stop craziness from happening.
		double m_dMinFrameTime;

		// The last frame delta that the render took.
		IOETimeDelta m_dFrameDelta;
	};

	//////////////////////////////////////////////////////////////////////////

	template <typename TApplicationType>
	class IOEEngine : public IOEEngineBase
	{
	public:
		IOEEngine();

		virtual IOE::Application::IOEApplicationBase*
		GetApplication() override final
		{
			return static_cast<IOE::Application::IOEApplicationBase*>(
				m_pApplication.get());
		}

		virtual const IOE::Application::IOEApplicationBase*
		GetApplication() const override final
		{
			return static_cast<const IOE::Application::IOEApplicationBase*>(
				m_pApplication.get());
		}

		virtual std::shared_ptr<IOE::Application::IOEApplicationBase>
		GetApplicationShared() override final
		{
			return std::static_pointer_cast<
				IOE::Application::IOEApplicationBase>(m_pApplication);
		}

		virtual std::shared_ptr<const IOE::Application::IOEApplicationBase>
		GetApplicationShared() const override final
		{
			return std::static_pointer_cast<
				IOE::Application::IOEApplicationBase>(m_pApplication);
		}

		FORCEINLINE_DEBUGGABLE virtual std::wstring
		GetBinPath() const override final;

		FORCEINLINE_DEBUGGABLE virtual std::wstring
		GetMediaPath() const override final;

	private:
		virtual std::int32_t EnginePreInit() override;
		virtual std::int32_t EngineInit() override;
		virtual std::int32_t EnginePostInit() override;
		virtual std::int32_t EngineLoop() override;
		virtual std::int32_t EnginePreStop() override;
		virtual std::int32_t EngineStop() override;
		virtual std::int32_t EnginePostStop() override;

	private:
		std::shared_ptr<TApplicationType> m_pApplication;
	};

	//////////////////////////////////////////////////////////////////////////

	extern std::unique_ptr<IOEEngineBase> g_pEngine;

	template <typename T>
	void MakeEngine()
	{
		g_pEngine = std::make_unique<IOE::Core::IOEEngine<T> >();
	}

//////////////////////////////////////////////////////////////////////////

#include "IOEEngine.inl"
}
}