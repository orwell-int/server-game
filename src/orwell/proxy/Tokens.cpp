#include "orwell/proxy/Tokens.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include <sstream>

namespace orwell
{
namespace proxy
{

static char const kNameQuotes = '"';

std::string ReadName(std::istringstream & ioStream)
{
	std::string aResult;
	ioStream >> aResult;
	char const aFirstChar = aResult.front();
	if (kNameQuotes == aFirstChar)
	{
		ORWELL_LOG_DEBUG("Try to parse string in quotes starting with '" << aResult << "'");
		std::string aFragment;
		while (ioStream.good())
		{
			ioStream >> aFragment;
			ORWELL_LOG_DEBUG(" add fragment '" << aFragment << "'");
			aResult += " " + aFragment;
			if (kNameQuotes == aFragment.back())
			{
				aResult = aResult.substr(1, aResult.length() - 2);
				break;
			}
		}
	}
	return aResult;
}

}
}
