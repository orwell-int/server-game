#include <url.hpp>

int main()
{
	bool r(false);
	orwell::com::url url;
	r = (url.toString() == "tcp://localhost:9000");
	if (!r) return 1;

	url.setProtocol("http");
	url.setHost("*");
	url.setPort(8080);

	r = (url.toString() == "http://*:8080");
	if (!r) return 1;

	return 0;
}

