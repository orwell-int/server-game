/* Urls used in rosk */

#pragma once

#include <string>

namespace orwell {
	namespace com {
		class url
		{
			std::string _protocol;
			std::string _host;
			uint32_t _port;

			std::string _url;

			void reset_url();

			public:
				explicit url();
				virtual ~url();

				void set_protocol(std::string const & iprotocol);
				void set_host(std::string const & ihost);
				void set_port(uint32_t iport);

				std::string const & get_url() const;

		};
	}
}

