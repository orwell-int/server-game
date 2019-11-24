#include "orwell/com/RawMessage.hpp"

#include <iostream>

namespace orwell
{
namespace com
{

RawMessage::RawMessage()
{
}

RawMessage::RawMessage(
		std::string const & iRoutId,
		std::string const & iType,
		std::string const & iPayload)
	: _routingId(iRoutId)
	, _type(iType)
	, _payload(iPayload)
{
}

}
}

