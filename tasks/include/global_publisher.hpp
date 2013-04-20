#pragma once

#include <orwell_message.hpp>
#include <string>

namespace orwell {
	namespace tasks {
		class global_publisher
		{
			orwell::com::orwell_message _orwell_message;
			std::string _tag;
			
			public:
				explicit global_publisher();
		};
	}
}

