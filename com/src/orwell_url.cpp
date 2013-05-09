#include <orwell_url.hpp>
#include <string>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace orwell::com;

url::url() : _protocol("tcp"), _port(9000), _host("localhost"), _url("tcp://localhost:9000")
{
}

url::~url()
{
}

void url::reset_url()
{
	_url.clear();
	_url += _protocol;
	_url += "://";
	_url += _host;
	_url += ":";
	_url += boost::lexical_cast<string>(_port);
}

void url::set_protocol(string const & iprotocol)
{
	if (!iprotocol.empty()) {
		_protocol = iprotocol;
		reset_url();
	}
}

void url::set_host(string const & ihost)
{
	if (!ihost.empty()) {
		_host = ihost;
		reset_url();
	}
}

void url::set_port(uint32_t iport)
{
	_port = iport;
	reset_url();
}

string const & url::get_url() const
{
	return _url;
}

