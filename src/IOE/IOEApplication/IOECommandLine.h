#pragma once

#include <map>
#include <vector>
#include <string>

#include <IOE/IOEExceptions/IOEExceptionList.h>

using std::map;
using std::pair;
using std::wstring;
using std::vector;
using std::wstringstream;

namespace IOE
{
namespace Application
{
	struct C_CommandLine
	{
		std::uint32_t nArgC;
		wchar_t **arrArgV;
	};

	class IOECommandLine
	{
	public:
		~IOECommandLine()
		{
			m_arrParsedCommandLine.clear();
		}

		FORCEINLINE const wstring &GetCommandLine() const
		{
			return m_szRawCommandLine;
		}

		FORCEINLINE const wstring &GetExecutionDir() const
		{
			return m_szExecutionDir;
		}

	public:
		//////////////////////////////////////////////////////////////////////////
		// @brief
		// Parses a string command line into a map of parameters and arguments
		//
		// @param string szExecutionpath The path to the executable currently
		// being
		//  executed. This is generally the first parameter on platforms.
		// @param string szCommandLine The entire command line, without the
		//  path to the binary currently being executed.
		// @outcome m_arrParsedCommandLine Contains the command line parameters
		//  and their values in a paired map.
		// @example
		//  Input: -param value -param2 value2 value3 -flag1 -flag2
		//  Output:
		//      Parameter | Arguments
		//      ----------+---------------------------
		//      "param"   | ["value"]
		//      "param2"  | ["value2", "value3"]
		//      "flag1"   | []
		//      "flag2"   | []
		//////////////////////////////////////////////////////////////////////////
		void ParseCommandLine(const wstring &szExecutionPath,
							  const wstring &szCommandLine);

		//////////////////////////////////////////////////////////////////////////
		// @brief
		// Parses a vector of command line arguments into a parsed map
		// structure
		//
		// @param string szExecutionpath The path to the executable currently
		// being
		//  executed. This is generally the first parameter on platforms.
		// @param vector<string> arrCommandLineParams A vector of string
		// parameters
		//  which have been passed on the command line without the execution
		//  path.
		// @outcome m_arrParsedCommandLine Contains the command line parameters
		//  and their values in a paired map.
		//////////////////////////////////////////////////////////////////////////
		void ParseCommandLine(const wstring &szExecutionPath,
							  const vector<wstring> &arrCommandLineParams);

	public:
		//////////////////////////////////////////////////////////////////////////
		// @brief
		// Tests whether a given argument exists within the command line
		//
		// @param string szArgument The argument to test for
		// @return bool Whether or not it exists
		//////////////////////////////////////////////////////////////////////////
		bool HasArgument(const wstring &szArgument) const;

		template <typename _T>
		_T GetArgumentValue(const wstring &szArgumentName,
							std::uint32_t nArgumentPosition) const
		{
			wstringstream sstream(GetArgumentValue<wstring>(
				IOE::Core::Algorithm::ToLower(szArgumentName),
				nArgumentPosition));
			_T value;
			if ((sstream >> value).rdstate() & std::ifstream::failbit)
			{
				THROW_IOE_CONVERSION_EXCEPTION(
					"Could not convert string to specified type",
					E_ERROR_NO_TYPE_IN_STRING);
			}
			return value;
		}

		template <>
		wstring
		GetArgumentValue<wstring>(const wstring &szArgumentName,
								  std::uint32_t nArgumentPosition) const
		{
			auto pIterator(m_arrParsedCommandLine.find(
				IOE::Core::Algorithm::ToLower(szArgumentName)));
			if (pIterator == m_arrParsedCommandLine.end())
			{
				THROW_IOE_BASE_EXCEPTION("Could not find the argument passed",
										 E_ERROR_INVALID_KEY)
			}
			auto tArgument(*pIterator);
			if (tArgument.second.size() > nArgumentPosition)
			{
				return tArgument.second.at(nArgumentPosition);
			}
			THROW_IOE_BASE_EXCEPTION("Could not find the argument position",
									 E_ERROR_INVALID_INDEX);
		}

	private:
		// Entire command line passed including execution path to the binary
		wstring m_szRawCommandLine;

		// The path to the current binary path being executed
		wstring m_szExecutionDir;

		//////////////////////////////////////////////////////////////////////////
		// An array of commands passed on the command line. The arguments can
		// have sub-arguments which will be stored in the array component of
		// this map.
		//
		// Example:
		//		executable.exe -argument1 param1 param2 param3
		//	The argument will be "argument1" and the sub-arguments will be
		// a vector containing "param1", "param2" and "param3" as strings.
		//////////////////////////////////////////////////////////////////////////
		map<wstring, vector<wstring> > m_arrParsedCommandLine;
	};

} // namespace Application
} // namespace IOE