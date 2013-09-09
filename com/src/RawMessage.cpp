#include <RawMessage.hpp>

#include <iostream>

using namespace std;

namespace orwell {
namespace com {


RawMessage::RawMessage(string const & iType, string const & iPayload) :
_type(iType), _payload(iPayload)
{
}

}} //namespaces
