/* Urls used in rosk */

#pragma once

#include <string>
#include <cstdint>
#include <inttypes.h>

namespace orwell {
namespace com {

class url
{
public:
	explicit url();
	virtual ~url();

	void setProtocol(std::string const & iProtocol);
	void setHost(std::string const & iHost);
	void setPort(uint16_t const iPort);

	void resetUrl();

	std::string const & toString() const;

private:
	std::string _protocol;
	std::string _host;
	uint16_t _port;

	std::string _url;

};
}
}
