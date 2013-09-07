/* This class stores most of the useful data of the server. */

#include "GlobalContext.hpp"

#include <iostream>
#include <zmq.hpp>

using namespace std;

namespace orwell {
namespace tasks {

GlobalContext::GlobalContext(com::Sender & iPublisher) : _publisher(iPublisher)
{
}

GlobalContext::~GlobalContext()
{
}

com::Sender & GlobalContext::getPublisher()
{
    return _publisher;
}

void GlobalContext::print()
{
	cout << "======GLOBAL CONTEXT======" << endl;
	for ( map<int, RobotContext>::iterator aIterOnRobots = _robots.begin();
			aIterOnRobots != _robots.end();
			++aIterOnRobots){
		cout << "====== ROBOT ID = " << aIterOnRobots->first << " ======" << endl;
		aIterOnRobots->second.print();
	}
}

int GlobalContext::addRobot()
{
    // find first free index available
	int aIndex = 0;
	cout << _robots.size() << endl;
	while ( _robots.find(aIndex) != _robots.end() ){
	    cout << "batman !" << endl;
		aIndex++;
	}

	// create RobotContext with that index
	RobotContext aRobotCtx(aIndex);


	_robots.insert( pair<int, RobotContext>( aIndex, aRobotCtx ) );
	cout << "Added new robot in context with ID " << aIndex << endl;
	cout << _robots.size() << endl;
	return aIndex;
}

}} // namespaces

