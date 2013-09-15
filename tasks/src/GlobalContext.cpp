/* This class stores most of the useful data of the server. */

#include "GlobalContext.hpp"

#include <iostream>
#include <zmq.hpp>

#include <boost/foreach.hpp>

using std::map;
using std::string;
using std::pair;

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

RobotContext & GlobalContext::accessRobot(string const & iRobotName)
{
    return _robots.at(iRobotName);
}
map<string, RobotContext> const & GlobalContext::getRobots()
{
    return _robots;
}
PlayerContext & GlobalContext::accessPlayer( string const & iPlayerName)
{
    return _players.at(iPlayerName);
}
map<string, PlayerContext> const & GlobalContext::getPlayers()
{
    return _players;
}

bool GlobalContext::addPlayer( string const & iName )
{
    bool aAddedPLayerSuccess;
    if ( _players.find(iName) != _players.end() )
    {
        LOG4CXX_WARN(_logger, "Player name (" << iName << ") is already in the player Map.");
	}
	else
	{
        //create playercontext and append
        PlayerContext aPlayerContext( iName );
        _players.insert( pair<string,PlayerContext>(iName, aPlayerContext) );
        LOG4CXX_DEBUG(_logger, "new PlayerContext added with internalId=" << iName);
        aAddedPLayerSuccess = true;
	}
    return aAddedPLayerSuccess;
}

bool GlobalContext::addRobot(string const & iName)
{
    bool aAddedRobotSuccess;
	if (_robots.find(iName) != _robots.end() )
	{
	    LOG4CXX_WARN(_logger, "Robot name (" << iName << ") is already in the robot Map.");
	}
	else
	{
        // create RobotContext with that index
        RobotContext aRobotCtx(iName);
        _robots.insert( pair<string, RobotContext>( iName, aRobotCtx ) );
        LOG4CXX_DEBUG(_logger, "new RobotContext added with internal ID=" << iName);
	    aAddedRobotSuccess = true;
	}
    return aAddedRobotSuccess;
}

string GlobalContext::getAvailableRobot()
{
    string aFoundRobot;

    //search for the first robot which is not already associated to a player
    map<string, RobotContext>::iterator aIterOnRobots;
    aIterOnRobots = _robots.begin();
    while ( aIterOnRobots != _robots.end()
            && !aIterOnRobots->second.getPlayerName().empty())
    {
        ++aIterOnRobots;
    }
    if ( !(aIterOnRobots == _robots.end()) )
    {
        aFoundRobot = aIterOnRobots->first;
    }
    return aFoundRobot;
}



}} // namespaces

