#include "IOEExceptionsPCH.h"
#include "IOEException_Enum.h"
#include "IOEException_Conversion.h"

namespace IOE
{
namespace Exceptions
{

	const EExceptionID IOEConversionException::GetMinErrorCodeRange() const
	{
		return EExceptionID::E_ERROR_CONVERSION_UNKNOWN;
	}

	const EExceptionID IOEConversionException::GetMaxErrorCodeRange() const
	{
		return EExceptionID::E_ERROR_CONVERSION_MAX;
	}

	IOEConversionException::IOEConversionException(
		const char *szWhat, const EExceptionID &nExceptionID,
		const char *szFile /*=nullptr*/, const int nLineNumber /*=-1*/)
		: IOEBaseException(
			  szWhat, nExceptionID, EExceptionID::E_ERROR_CONVERSION_UNKNOWN,
			  EExceptionID::E_ERROR_CONVERSION_MAX, szFile, nLineNumber)
	{
	}

} // namespace Exceptions
} // namespace IOL