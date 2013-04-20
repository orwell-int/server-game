#include <orwell_url.hpp>

int main()
{
	bool r(false);
	orwell::com::url url;
	r = (url.get_url() == "tcp://localhost:9000");
	if (!r) return 1;

	url.set_protocol("http");
	url.set_host("*");
	url.set_port(8080);

	r = (url.get_url() == "http://*:8080");
	if (!r) return 1;
	
	return 0;
}

