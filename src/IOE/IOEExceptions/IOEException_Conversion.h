#pragma once

#include "IOEException_Base.h"

#define THROW_IOE_CONVERSION_EXCEPTION(text, errorid)                         \
	throw IOE::Exceptions::IOEConversionException(                            \
		text, IOE::Exceptions::EExceptionID::errorid, __FILE__, __LINE__);

#define THROW_IOE_CONVERSION_EXCEPTION_VARGS(text, errorid, ...)              \
	{                                                                         \
		char _exc_buffer[IOE::Exceptions::gs_uExceptionBufferSize];           \
		sprintf_s(_exc_buffer, IOE::Exceptions::gs_uExceptionBufferSize,      \
				  text, __VA_ARGS__);                                         \
		throw IOE::Exceptions::IOEConversionException(                        \
			_exc_buffer, IOE::Exceptions::EExceptionID::errorid, __FILE__,    \
			__LINE__);                                                        \
	}

namespace IOE
{
namespace Exceptions
{

	enum class EExceptionID : std::uint16_t;

	class IOEConversionException : public IOEBaseException
	{
	public:
		IOEConversionException(const char *szWhat,
							   const EExceptionID &nExceptionID,
							   const char *szFile	= nullptr,
							   const int nLineNumber = -1);

	protected:
		virtual const EExceptionID GetMinErrorCodeRange() const;
		virtual const EExceptionID GetMaxErrorCodeRange() const;
	};

} // namespace Exceptions
} // namespace IOE