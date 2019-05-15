#pragma once

#include "IOEException_Base.h"

#include <cstdint>

#define THROW_IOE_RENDERER_EXCEPTION(text, errorid)                           \
	throw IOE::Exceptions::IOERendererException(                              \
		text, IOE::Exceptions::EExceptionID::errorid, __FILE__, __LINE__);

#define THROW_IOE_RENDERER_EXCEPTION_VARGS(text, errorid, ...)                \
	{                                                                         \
		char _exc_buffer[IOE::Exceptions::gs_uExceptionBufferSize];           \
		sprintf_s(_exc_buffer, IOE::Exceptions::gs_uExceptionBufferSize,      \
				  text, __VA_ARGS__);                                         \
		throw IOE::Exceptions::IOERendererException(                          \
			_exc_buffer, IOE::Exceptions::EExceptionID::errorid, __FILE__,    \
			__LINE__);                                                        \
	}

namespace IOE
{
namespace Exceptions
{
	enum class EExceptionID : std::uint16_t;

	class IOERendererException : public IOEBaseException
	{
	public:
		IOERendererException(const char *szWhat,
							 const EExceptionID &nExceptionID,
							 const char *szFile	= nullptr,
							 const int nLineNumber = -1);

	protected:
		virtual const EExceptionID GetMinErrorCodeRange() const;
		virtual const EExceptionID GetMaxErrorCodeRange() const;
	};
} // namespace Exceptions
} // namespace IOE