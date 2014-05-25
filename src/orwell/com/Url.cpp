#include "orwell/com/Url.hpp"
#include <string>
#include <boost/lexical_cast.hpp>

#include "orwell/support/GlobalLogger.hpp"

using std::string;
using namespace orwell::com;

Url::Url() : _protocol("tcp"), _host("localhost"),_port(9000), _url("tcp://localhost:9000")
{
}

Url::~Url()
{
}

void Url::resetUrl()
{
	_url.clear();
	_url += _protocol;
	_url += "://";
	_url += _host;
	_url += ":";
	_url += boost::lexical_cast<string>(_port);
	//ORWELL_LOG_DEBUG("url = '" << _url << "'");
}

void Url::setProtocol(string const & iProtocol)
{
	if (!iProtocol.empty())
	{
		_protocol = iProtocol;
		resetUrl();
	}
}

void Url::setHost(string const & iHost)
{
	if (!iHost.empty())
	{
		_host = iHost;
		resetUrl();
	}
}

void Url::setPort(uint16_t const iPort)
{
	_port = iPort;
	resetUrl();
}

string const & Url::toString() const
{
	return _url;
}

std::string const & Url::getHost() const
{
	return _host;
}

uint16_t const & Url::getPort() const
{
	return _port;
}

