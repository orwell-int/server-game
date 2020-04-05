#include "orwell/proxy/OutputMode.hpp"

#include <ostream>

namespace orwell
{
namespace proxy
{

std::ostream & operator<<(std::ostream & ioStream, OutputMode const iOutputMode)
{
	switch (iOutputMode)
	{
		case OutputMode::kText:
		{
			ioStream << "text";
			break;
		}
		case OutputMode::kJson:
		{
			ioStream << "json";
			break;
		}
	}
	return ioStream;
}

}
}
