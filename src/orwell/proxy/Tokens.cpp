#include "orwell/proxy/Tokens.hpp"

#include <sstream>

namespace orwell
{
namespace proxy
{

static char const kNameQuotes = '"';

std::string ReadName(std::istringstream & ioStream)
{
	std::string aResult;
	std::string aArg;
	ioStream >> aArg;
	size_t aLength = aArg.size();
	if (0 < aLength)
	{
		if (kNameQuotes == aArg.front())
		{
			aArg = aArg.substr(1);
			--aLength;
			bool aContinue = true;
			bool aFirst = true;
			while (aContinue)
			{
				if (0 < aLength)
				{
					aContinue = (aArg.back() != kNameQuotes);
					if (not aContinue)
					{
						aArg = aArg.substr(0, aLength - 1);
					}
				}
				if (not aFirst)
				{
					aResult += " ";
				}
				else
				{
					aFirst = false;
				}
				aResult += aArg;
				if ((aContinue) and (not ioStream.eof()))
				{
					ioStream >> aArg;
					aLength = aArg.size();
				}
			}
		}
		else
		{
			aResult = aArg;
		}
	}
	return aResult;
}

}
}
