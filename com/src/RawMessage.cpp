#include <RawMessage.hpp>

#include <iostream>

using namespace std;

namespace orwell {
namespace com {


RawMessage::RawMessage(std::string const & iRoutId, std::string const & iType, std::string const & iPayload) :
_routingId(iRoutId), _type(iType), _payload(iPayload)
{
}

}} //namespaces
