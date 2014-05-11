#include "orwell/com/url.hpp"
#include <string>
#include <boost/lexical_cast.hpp>

#include "orwell/support/GlobalLogger.hpp"

using std::string;
using namespace orwell::com;

url::url() : _protocol("tcp"), _host("localhost"),_port(9000), _url("tcp://localhost:9000")
{
}

url::~url()
{
}

void url::resetUrl()
{
	_url.clear();
	_url += _protocol;
	_url += "://";
	_url += _host;
	_url += ":";
	_url += boost::lexical_cast<string>(_port);
	//ORWELL_LOG_DEBUG("url = '" << _url << "'");
}

void url::setProtocol(string const & iProtocol)
{
	if (!iProtocol.empty())
	{
		_protocol = iProtocol;
		resetUrl();
	}
}

void url::setHost(string const & iHost)
{
	if (!iHost.empty())
	{
		_host = iHost;
		resetUrl();
	}
}

void url::setPort(uint16_t const iPort)
{
	_port = iPort;
	resetUrl();
}

string const & url::toString() const
{
	return _url;
}

