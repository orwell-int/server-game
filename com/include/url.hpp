/* Urls used in rosk */

#pragma once

#include <string>

namespace orwell {
	namespace com {
		class url
		{
			public:
				explicit url();
				virtual ~url();

				void setProtocol(std::string const & iprotocol);
				void setHost(std::string const & ihost);
				void setPort(uint32_t iport);

				void resetUrl();

				std::string const & toString() const;

            private:
                std::string _protocol;
                std::string _host;
                uint32_t _port;

                std::string _url;

		};
	}
}

