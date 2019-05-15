#include "IOEExceptionsPCH.h"
#include "IOEException_Enum.h"
#include "IOEException_Type.h"

namespace IOE
{
namespace Exceptions
{

	const EExceptionID IOETypeException::GetMinErrorCodeRange() const
	{
		return EExceptionID::E_ERROR_TYPE_UNKNOWN;
	}

	const EExceptionID IOETypeException::GetMaxErrorCodeRange() const
	{
		return EExceptionID::E_ERROR_TYPE_MAX;
	}

	IOETypeException::IOETypeException(const char *szWhat,
									   const EExceptionID &nExceptionID,
									   const char *szFile /*=nullptr*/,
									   const int nLineNumber /*=-1*/)
		: IOEBaseException(szWhat, nExceptionID,
						   EExceptionID::E_ERROR_TYPE_UNKNOWN,
						   EExceptionID::E_ERROR_TYPE_MAX, szFile, nLineNumber)
	{
	}
}
}