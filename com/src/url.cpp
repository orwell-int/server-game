#include <url.hpp>
#include <string>
#include <boost/lexical_cast.hpp>

using namespace std;
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
}

void url::setProtocol(string const & iprotocol)
{
	if (!iprotocol.empty()) {
		_protocol = iprotocol;
		resetUrl();
	}
}

void url::setHost(string const & ihost)
{
	if (!ihost.empty()) {
		_host = ihost;
		resetUrl();
	}
}

void url::setPort(uint32_t iport)
{
	_port = iport;
	resetUrl();
}

string const & url::toString() const
{
	return _url;
}

