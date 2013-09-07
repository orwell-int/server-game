/* This class stores most of the useful data of the server. */

#pragma once

#include <map>
#include <RobotContext.hpp>
#include <Sender.hpp>

namespace orwell {
namespace tasks {

class GlobalContext
{
public:
    // constructor
    GlobalContext( com::Sender & iPublisher );

    // destructor
    ~GlobalContext();

    void print();

    com::Sender & getPublisher();

    // add an empty robotContext to the map, and gives it the first integer that is not already in the keys of the map as an ID.
    int addRobot();

//		robotContext get_robot(unsigned int robot_index);

private:
    com::Sender & _publisher;

    // Each connected robot has a robotContext in this map. The key is the robot ID.
    std::map<int, RobotContext> _robots;
};

}} //end namespace
