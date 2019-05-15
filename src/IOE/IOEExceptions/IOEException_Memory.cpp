#include "IOEExceptionsPCH.h"
#include "IOEException_Memory.h"

namespace IOE
{
namespace Exceptions
{

	const EExceptionID IOEMemoryException::GetMinErrorCodeRange() const
	{
		return EExceptionID::E_ERROR_MEMORY_UNKNOWN;
	}

	const EExceptionID IOEMemoryException::GetMaxErrorCodeRange() const
	{
		return EExceptionID::E_ERROR_MEMORY_MAX;
	}

	IOEMemoryException::IOEMemoryException(const char *szWhat,
										   const EExceptionID &nExceptionID,
										   const char *szFile /*=nullptr*/,
										   const int nLineNumber /*=-1*/)
		: IOEBaseException(
			  szWhat, nExceptionID, EExceptionID::E_ERROR_MEMORY_UNKNOWN,
			  EExceptionID::E_ERROR_MEMORY_MAX, szFile, nLineNumber)
	{
	}
}
}