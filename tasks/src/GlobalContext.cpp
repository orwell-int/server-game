/* This class stores most of the useful data of the server. */

#include "GlobalContext.hpp"

#include <iostream>
#include <zmq.hpp>

#include <boost/foreach.hpp>

using namespace std;

namespace orwell {
namespace tasks {

GlobalContext::GlobalContext(com::Sender & iPublisher) :
_publisher(iPublisher), _logger(log4cxx::Logger::getLogger("orwell.log"))
{
}

GlobalContext::~GlobalContext()
{
}

com::Sender & GlobalContext::getPublisher()
{
    return _publisher;
}

map<string, RobotContext> & GlobalContext::accessRobots()
{
    return _robots;
}
map<string, RobotContext> const & GlobalContext::getRobots()
{
    return _robots;
}
map<string, PlayerContext> & GlobalContext::accessPlayers()
{
    return _players;
}
map<string, PlayerContext> const & GlobalContext::getPlayers()
{
    return _players;
}

bool GlobalContext::addPlayer( string const & iName )
{
    if ( _players.find(iName) != _players.end() ){
        LOG4CXX_WARN(_logger, "Player name (" << iName << ") is already in the player Map.");
		return false;
	}

    //create playercontext and append
    PlayerContext aPlayerContext( iName );
    _players.insert( pair<string,PlayerContext>(iName, aPlayerContext) );
    LOG4CXX_DEBUG(_logger, "new PlayerContext added with internalId=" << iName);
    return true;
}

bool GlobalContext::addRobot(string const & iName)
{
	if (_robots.find(iName) != _robots.end() ){
	    LOG4CXX_WARN(_logger, "Robot name (" << iName << ") is already in the robot Map.");
		return false;
	}

	// create RobotContext with that index
	RobotContext aRobotCtx(iName);
	_robots.insert( pair<string, RobotContext>( iName, aRobotCtx ) );
    LOG4CXX_DEBUG(_logger, "new RobotContext added with internal ID=" << iName);
    return true;
}

bool GlobalContext::giveRobot(string const & iName)
{
    //search for the first robot which is not already associated to a player
    std::map<std::string, RobotContext>::iterator aIterOnRobots;
    aIterOnRobots = _robots.begin();
    while ( !aIterOnRobots->second.getControlledBy().empty() )
    {
        ++aIterOnRobots;
    }

    if (aIterOnRobots == _robots.end())
    {
        LOG4CXX_WARN(_logger, "Cannot associate robot : no more free robot");
        return false;
    }
    else
    {
        aIterOnRobots->second.setControlledBy(iName);
        LOG4CXX_INFO(_logger, "Player " << iName << " now controls robot " << aIterOnRobots->first);
        return true;
    }

}



}} // namespaces

