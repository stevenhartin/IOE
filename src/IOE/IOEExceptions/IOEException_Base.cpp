#include "IOEExceptionsPCH.h"
#include "IOEException_Base.h"
#include "IOEException_Enum.h"

#include <IOE/IOECore/IOEHeaders.h>

namespace IOE
{
namespace Exceptions
{

	IOEBaseException::IOEBaseException(const char *szWhat,
									   const EExceptionID &nExceptionID,
									   const EExceptionID &nMinID,
									   const EExceptionID &nMaxID,
									   const char *szFile /*=nullptr*/,
									   const int nLineNumber /*=-1*/)
		: m_nExceptionID(nExceptionID)
	{
		IOE_ASSERT(m_nExceptionID < nMaxID && m_nExceptionID >= nMinID,
				   "Invalid error code");

		if (m_nExceptionID >= nMaxID || m_nExceptionID < nMinID)
		{
			m_nExceptionID = nMaxID;
		}

		std::stringstream sstream;
		sstream << "IOE EXCEPTION (E: "
				<< static_cast<std::int16_t>(m_nExceptionID) << ")";
		if (nullptr != szFile)
		{
			sstream << " '" << szFile << "'";
		}
		if (nLineNumber >= 0)
		{
			sstream << " (" << nLineNumber << ")";
		}
		sstream << "\n" << szWhat;
		m_szWhat = sstream.str();
	}

	const char *IOEBaseException::what() const
	{
		return m_szWhat.c_str();
	}

	const EExceptionID IOEBaseException::GetMinErrorCodeRange() const
	{
		return EExceptionID::E_ERROR_UNKNOWN;
	}

	const EExceptionID IOEBaseException::GetMaxErrorCodeRange() const
	{
		return EExceptionID::E_ERROR_MAX;
	}

} // namespace Exceptions
} // namespace IOL