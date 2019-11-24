#include "orwell/com/Url.hpp"
#include <string>
#include <boost/lexical_cast.hpp>

namespace orwell
{
namespace com
{

Url::Url(
		std::string const & iProtocol,
		std::string const & iHost,
		uint16_t const iPort)
	: m_protocol(iProtocol)
	, m_host(iHost)
	, m_port(iPort)
{
	resetUrl();
}

Url::~Url()
{
}

void Url::resetUrl()
{
	m_url.clear();
	m_url += m_protocol;
	m_url += "://";
	m_url += m_host;
	m_url += ":";
	m_url += boost::lexical_cast<std::string>(m_port);
}

void Url::setProtocol(std::string const & iProtocol)
{
	if (!iProtocol.empty())
	{
		m_protocol = iProtocol;
		resetUrl();
	}
}

void Url::setHost(std::string const & iHost)
{
	if (!iHost.empty())
	{
		m_host = iHost;
		resetUrl();
	}
}

void Url::setPort(uint16_t const iPort)
{
	m_port = iPort;
	resetUrl();
}

std::string const & Url::toString() const
{
	return m_url;
}

std::string const & Url::getHost() const
{
	return m_host;
}

uint16_t const & Url::getPort() const
{
	return m_port;
}

}
}
