#include "RobotContext.hpp"
#include <iostream>

using namespace std;

namespace orwell {
namespace tasks {

RobotContext::RobotContext(int iId) : _id(iId)
{

}

RobotContext::~RobotContext()
{

}

int RobotContext::getId(){
    return _id;
}

void RobotContext::print()
{
	cout << "this is a robot" << endl;
}

}} // namespaces


