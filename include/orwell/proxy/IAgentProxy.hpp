
#pragma once

#include <string>
#include <stdint.h>

namespace orwell
{
namespace proxy
{

/// Class that gives access to commands to be run from an agent.
class IAgentProxy
{
public :
	virtual ~IAgentProxy();

	/// \return
	///  True if and only if the command was successfully parsed.
	///
	virtual bool step(
			std::string const & iCommand,
			std::string & ioReply) = 0;
};

}
}
