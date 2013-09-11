/* This class stores the information about a robot that is connected to the server */

#pragma once

#include <string>

namespace orwell {
namespace tasks {

class RobotContext
{
	public:
		RobotContext(std::string const & iName);
		~RobotContext();

        void setControlledBy(std::string const & iName);
		std::string const &  getName() const;
		std::string const &  getControlledBy() const;

    private:
        std::string _name;
        std::string _controlledBy;
};

}} //end namespace
