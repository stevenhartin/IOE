#include "IOEExceptionsPCH.h"
#include "IOEException_Renderer.h"

namespace IOE
{
namespace Exceptions
{

	const EExceptionID IOERendererException::GetMinErrorCodeRange() const
	{
		return EExceptionID::E_ERROR_RENDERER_UNKNOWN;
	}

	const EExceptionID IOERendererException::GetMaxErrorCodeRange() const
	{
		return EExceptionID::E_ERROR_RENDERER_MAX;
	}

	IOERendererException::IOERendererException(
		const char *szWhat, const EExceptionID &nExceptionID,
		const char *szFile /*=nullptr*/, const int nLineNumber /*=-1*/)
		: IOEBaseException(
			  szWhat, nExceptionID, EExceptionID::E_ERROR_RENDERER_UNKNOWN,
			  EExceptionID::E_ERROR_RENDERER_MAX, szFile, nLineNumber)
	{
	}
}
}