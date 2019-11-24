#pragma once

#include <string>
#include <map>

#include <log4cxx/logger.h>

#include <iostream>

namespace orwell
{
namespace support
{
class GlobalLogger
{
public :
	/// The first logger created will be the one used.
	/// \param iName
	///  Used to identify the logger.
	/// \param iOutput
	///  Name of the file used to write the logs to.
	/// \param iDebug
	///  Activate all logs if true (only INFO and above allowed otherwise).
	static void Create(
			std::string const & iName,
			std::string const & iOutput,
			bool const iDebug = false);

	static log4cxx::LoggerPtr GetActiveLogger();

	static void Clear();

private :
	static log4cxx::LoggerPtr m_ActiveLogger;
	struct Pimpl;
	static Pimpl * m_Garbage;
};

#define ORWELL_LOG_TRACE(content) LOG4CXX_TRACE(::orwell::support::GlobalLogger::GetActiveLogger(), content)
#define ORWELL_LOG_DEBUG(content) LOG4CXX_DEBUG(::orwell::support::GlobalLogger::GetActiveLogger(), content)
#define ORWELL_LOG_INFO(content) LOG4CXX_INFO(::orwell::support::GlobalLogger::GetActiveLogger(), content)
#define ORWELL_LOG_WARN(content) LOG4CXX_WARN(::orwell::support::GlobalLogger::GetActiveLogger(), content)
#define ORWELL_LOG_ERROR(content) LOG4CXX_ERROR(::orwell::support::GlobalLogger::GetActiveLogger(), content)
#define ORWELL_LOG_FATAL(content) LOG4CXX_FATAL(::orwell::support::GlobalLogger::GetActiveLogger(), content)

}
}

