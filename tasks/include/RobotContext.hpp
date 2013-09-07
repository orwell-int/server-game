/* This class stores the information about a robot that is connected to the server */

#pragma once

namespace orwell {
namespace tasks {

class RobotContext
{
	public:
		// constructor
		RobotContext(int iId);

 		// destructor
		~RobotContext();

        // getters
		int getId();


		void print();

    private:
        int _id;
};

}} //end namespace
