#pragma once

#include <sstream>
#include <string>
#include <exception>
#include <cstdint>
#include <IOE/IOECore/IOEDefines.h>

#define THROW_IOE_BASE_EXCEPTION(text, errorid)                               \
	throw IOE::Exceptions::IOEBaseException(                                  \
		text, IOE::Exceptions::EExceptionID::errorid,                         \
		IOE::Exceptions::EExceptionID::E_ERROR_UNKNOWN,                       \
		IOE::Exceptions::EExceptionID::E_ERROR_MAX, __FILE__, __LINE__);

#define THROW_IOE_BASE_EXCEPTION_VARGS(text, errorid, ...)                    \
	{                                                                         \
		char _exc_buffer[IOE::Exceptions::gs_uExceptionBufferSize];           \
		sprintf_s(_exc_buffer, IOE::Exceptions::gs_uExceptionBufferSize,      \
				  text, __VA_ARGS__);                                         \
		throw IOE::Exceptions::IOEBaseException(                              \
			_exc_buffer, IOE::Exceptions::EExceptionID::errorid,              \
			IOE::Exceptions::EExceptionID::E_ERROR_UNKNOWN,                   \
			IOE::Exceptions::EExceptionID::E_ERROR_MAX, __FILE__, __LINE__);  \
	}

namespace IOE
{
namespace Exceptions
{
	//////////////////////////////////////////////////////////////////////////

	static const std::uint32_t gs_uExceptionBufferSize = 10 * 1024;

	//////////////////////////////////////////////////////////////////////////

	enum class EExceptionID : std::uint16_t;

	class IOEBaseException : public std::exception
	{
	public:
		IOEBaseException(const char *szWhat, const EExceptionID &nExceptionID,
						 const EExceptionID &nMinID,
						 const EExceptionID &nMaxID,
						 const char *szFile	= nullptr,
						 const int nLineNumber = -1);

		virtual const char *what() const;
		FORCEINLINE EExceptionID GetErrorCode() const
		{
			return m_nExceptionID;
		}

	protected:
		virtual const EExceptionID GetMinErrorCodeRange() const;
		virtual const EExceptionID GetMaxErrorCodeRange() const;

	private:
		std::string m_szWhat;
		EExceptionID m_nExceptionID;
	};

} // namespace Exceptions
} // namespace IOE