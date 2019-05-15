#include <IOE/IOECore/IOEHeaders.h>

#include "IOECommandLine.h"

#include EXTERNAL_PLATFORM_INCLUDE(IOECore, IOEPlatformStatics_Platform.h)

#include <IOE/IOECore/Algorithms/IOETokeniser.h>
#include <IOE/IOECore/Algorithms/IOEStringHelpers.h>

namespace IOE
{
namespace Application
{

	bool IOECommandLine::HasArgument(const wstring &szArgument) const
	{
		return (m_arrParsedCommandLine.find(IOE::Core::Algorithm::ToLower(
					szArgument)) != m_arrParsedCommandLine.end());
	}

	void IOECommandLine::ParseCommandLine(const wstring &szExecutionPath,
										  const wstring &szCommandLine)
	{
		m_szExecutionDir = szExecutionPath;
		IOE::Core::Algorithm::FixupPath(m_szExecutionDir);
		m_szExecutionDir = IOE::Core::Algorithm::GetBasePath(m_szExecutionDir);
		m_szRawCommandLine = szExecutionPath + szCommandLine;

		// Tokenize string into array of strings separated via space
		auto tCurrentPair = m_arrParsedCommandLine.end();
		IOE::Core::Algorithm::TTokeniser<wchar_t> tTokens(szCommandLine, ' ');

		for (std::wstring szTempCopy : tTokens)
		{
			// Remove whitespace around token
			// std::wstring szTempCopy(szToken);
			IOE::Core::Algorithm::Trim(szTempCopy);
			szTempCopy = IOE::Core::Algorithm::ToLower(szTempCopy);
			if (szTempCopy.length() && szTempCopy.at(0) == L'-')
			{
				// This is a parameter
				auto tRetVal(m_arrParsedCommandLine.emplace(
					wstring(szTempCopy.begin() + 1, szTempCopy.end()),
					vector<wstring>()));
				if (tRetVal.second)
				{
					tCurrentPair = tRetVal.first;
				}
				else
				{
					tCurrentPair = m_arrParsedCommandLine.end();
				}
			}
			else if (tCurrentPair != m_arrParsedCommandLine.end())
			{
				// This is an argument, append to the argument vector for the
				// current parameter - argument pair.
				tCurrentPair->second.push_back(szTempCopy);
			}
		}
	}

	void IOECommandLine::ParseCommandLine(
		const wstring &szExecutionPath,
		const vector<wstring> &arrCommandLineParams)
	{
		m_szExecutionDir = szExecutionPath;
		IOE::Core::Algorithm::FixupPath(m_szExecutionDir);
		m_szExecutionDir = IOE::Core::Algorithm::GetBasePath(m_szExecutionDir);
		m_szRawCommandLine = L"\"" + szExecutionPath + L"\" " +
			IOE::Core::Algorithm::Join<std::wstring>(
								 arrCommandLineParams.begin(),
								 arrCommandLineParams.end(), L" ");

		auto tCurrentPair = m_arrParsedCommandLine.end();

		for (auto szTempCopy : arrCommandLineParams)
		{
			// Remove whitespace around token
			IOE::Core::Algorithm::Trim(szTempCopy);
			szTempCopy = IOE::Core::Algorithm::ToLower(szTempCopy);
			if (szTempCopy.length() && szTempCopy.at(0) == '-')
			{
				// This is a parameter
				auto tRetPair(m_arrParsedCommandLine.emplace(
					wstring(szTempCopy.begin() + 1, szTempCopy.end()),
					vector<wstring>()));

				if (tRetPair.second)
				{
					tCurrentPair = tRetPair.first;
				}
				else
				{
					tCurrentPair = m_arrParsedCommandLine.end();
				}
			}
			else if (tCurrentPair != m_arrParsedCommandLine.end())
			{
				// This is an argument, append to the argument vector for the
				// current parameter - argument pair.
				tCurrentPair->second.push_back(szTempCopy);
			}
		}
	}
} // namespace Application
} // namespace IOE