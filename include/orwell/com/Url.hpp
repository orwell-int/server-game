/* Urls */

#pragma once

#include <string>
#include <cstdint>
#include <inttypes.h>

namespace orwell
{
namespace com
{

class Url
{
public:
	explicit Url(
			std::string const & iProtocol,
			std::string const & iHost,
			uint16_t const iPort);
	virtual ~Url();

	void setProtocol(std::string const & iProtocol);
	void setHost(std::string const & iHost);
	void setPort(uint16_t const iPort);

	void resetUrl();

	std::string const & toString() const;

	std::string const & getHost() const;
	uint16_t const & getPort() const;

private:
	std::string m_protocol;
	std::string m_host;
	uint16_t m_port;

	std::string m_url;

};
}
}

