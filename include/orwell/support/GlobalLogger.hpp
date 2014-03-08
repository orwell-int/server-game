#pragma once

#include <string>
#include <map>

#include <log4cxx/logger.h>

namespace orwell
{
namespace support
{
class GlobalLogger
{
public :
	/// The first logger created with be the one used.
	/// \param iName
	///  Used to identify the logger.
	/// \param iOutput
	///  Nane of the file used to write the logs to.
	/// \param iDebug
	///  Activate all logs if true (only INFO and above allowed otherwise).
	GlobalLogger(
			std::string const & iName,
			std::string const & iOutput,
			bool const iDebug = false);

	/// Throw if the name of the logger is not valid.
	/// \param iName
	///  Name of the logger (provided in a #GlobalLogger constructor.
	static void SwitchToLogger(std::string const & iName);

	static log4cxx::LoggerPtr GetActiveLogger();

	static void Clear();

private :
	static std::map< std::string, log4cxx::LoggerPtr > m_Loggers;
	static log4cxx::LoggerPtr _ActiveLogger;
};

#define ORWELL_LOG_TRACE(content) LOG4CXX_TRACE(::orwell::support::GlobalLogger::GetActiveLogger(), content)
#define ORWELL_LOG_DEBUG(content) LOG4CXX_DEBUG(::orwell::support::GlobalLogger::GetActiveLogger(), content)
#define ORWELL_LOG_INFO(content) LOG4CXX_INFO(::orwell::support::GlobalLogger::GetActiveLogger(), content)
#define ORWELL_LOG_WARN(content) LOG4CXX_WARN(::orwell::support::GlobalLogger::GetActiveLogger(), content)
#define ORWELL_LOG_ERROR(content) LOG4CXX_ERROR(::orwell::support::GlobalLogger::GetActiveLogger(), content)
#define ORWELL_LOG_FATAL(content) LOG4CXX_FATAL(::orwell::support::GlobalLogger::GetActiveLogger(), content)

}
}

