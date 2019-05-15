#include "IOEEngine.h"
#include <IOE/IOEApplication/IOEApplication.h>
#include <IOE/IOECore/IOEManagerContainer.h>

namespace IOE
{
namespace Core
{

	//////////////////////////////////////////////////////////////////////////

	std::unique_ptr<IOEEngineBase> g_pEngine;

	//////////////////////////////////////////////////////////////////////////

	IOEEngineBase::IOEEngineBase()
		: m_ePhase(EEnginePhase::PreInit)
		, m_uRequestingExit(0)
		, m_dFrameDelta(0.0f)
	{
		SetMaxFPS(60.0f);
		SetMinFPS(10.0f);
	}

	//////////////////////////////////////////////////////////////////////////

	std::int32_t IOEEngineBase::Run()
	{
		std::int32_t nReturnResult(0);
		try
		{
			SetEnginePhase(EEnginePhase::PreInit);
			nReturnResult = EnginePreInit();
			if (nReturnResult != 0)
				return nReturnResult;

			SetEnginePhase(EEnginePhase::Init);
			nReturnResult = EngineInit();
			if (nReturnResult != 0)
				return nReturnResult;

			SetEnginePhase(EEnginePhase::PostInit);
			nReturnResult = EnginePostInit();
			if (nReturnResult != 0)
				return nReturnResult;

			SetEnginePhase(EEnginePhase::Loop);
			nReturnResult = EngineLoop();
			if (nReturnResult != 0)
				return nReturnResult;

			SetEnginePhase(EEnginePhase::PreStop);
			nReturnResult = EnginePreStop();
			if (nReturnResult != 0)
				return nReturnResult;

			SetEnginePhase(EEnginePhase::Stop);
			nReturnResult = EngineStop();
			if (nReturnResult != 0)
				return nReturnResult;

			SetEnginePhase(EEnginePhase::PostStop);
			nReturnResult = EnginePostStop();
		}
		catch (IOE::Exceptions::IOEBaseException &e)
		{
			std::cerr << "Error: Unhandled exception:\n" << e.what()
					  << std::endl;
			RequestExit();
			std::cin.get();
			nReturnResult = static_cast<int32_t>(e.GetErrorCode());
		}

		return nReturnResult;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEEngineBase::RequestExit()
	{
		m_uRequestingExit = true;
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Core
} // namespace IOE