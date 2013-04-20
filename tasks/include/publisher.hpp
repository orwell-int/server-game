#pragma once

#include <string>
#include <orwell_url.hpp>

namespace zmq {
	class context_t;
	class socket_t;
}

namespace orwell {

	namespace com {
		class orwell_message;
	}

	namespace tasks {
		class publisher
		{
			std::string _tag;
			com::url _url;
			zmq::context_t * _zmq_context;
			zmq::socket_t * _zmq_socket;

			public:
				explicit publisher();
				publisher(std::string const & itag);
				publisher(std::string const & itag, orwell::com::url const & iurl);
				publisher(std::string const & itag, std::string const & ihost, uint32_t iport);

				void bind();
				void publish(orwell::com::orwell_message & iorwell_message);

				std::string const & get_tag() const;
				void set_tag(std::string const & itag);

				std::string const & get_url() const;

				virtual ~publisher();
		};
	}
}

