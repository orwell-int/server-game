#pragma once

#include <string>
#include <memory>
#include <google/protobuf/message.h>

namespace orwell {
namespace com {

class RawMessage
{

public:
    std::string _type;
    std::string _payload;

    RawMessage(std::string const & iType, std::string const & iPayload);

};

}} //namespace
