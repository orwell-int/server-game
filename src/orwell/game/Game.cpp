/* This class stores most of the useful data of the server. */

#include "Game.hpp"

#include "Robot.hpp"
#include "Player.hpp"
#include "Sender.hpp"

#include <iostream>
#include <zmq.hpp>

#include <boost/foreach.hpp>

using std::map;
using std::string;
using std::pair;

namespace orwell {
namespace game {

Game::Game(std::shared_ptr< com::Sender > iPublisher) :
_publisher(iPublisher), _logger(log4cxx::Logger::getLogger("orwell.log"))
{
}

Game::~Game()
{
}

std::shared_ptr< com::Sender > Game::getPublisher()
{
    return _publisher;
}

Robot & Game::accessRobot(string const & iRobotName)
{
    return _robots.at(iRobotName);
}
map<string, Robot> const & Game::getRobots()
{
    return _robots;
}
Player & Game::accessPlayer( string const & iPlayerName)
{
    return _players.at(iPlayerName);
}
map<string, Player> const & Game::getPlayers()
{
    return _players;
}

bool Game::addPlayer( string const & iName )
{
    bool aAddedPLayerSuccess;
    if ( _players.find(iName) != _players.end() )
    {
        LOG4CXX_WARN(_logger, "Player name (" << iName << ") is already in the player Map.");
	}
	else
	{
        //create playercontext and append
        Player aPlayerContext( iName );
        _players.insert( pair<string,Player>(iName, aPlayerContext) );
        LOG4CXX_DEBUG(_logger, "new PlayerContext added with internalId=" << iName);
        aAddedPLayerSuccess = true;
	}
    return aAddedPLayerSuccess;
}

bool Game::addRobot(string const & iName)
{
    bool aAddedRobotSuccess;
	if (_robots.find(iName) != _robots.end())
	{
	    LOG4CXX_WARN(_logger, "Robot name (" << iName << ") is already in the robot Map.");
	}
	else
	{
        // create RobotContext with that index
        Robot aRobotCtx(iName);
        _robots.insert( pair<string, Robot>( iName, aRobotCtx ) );
        LOG4CXX_DEBUG(_logger, "new RobotContext added with internal ID=" << iName);
	    aAddedRobotSuccess = true;
	}
    return aAddedRobotSuccess;
}

string Game::getAvailableRobot()
{
    string aFoundRobot;

    //search for the first robot which is not already associated to a player
    map<string, Robot>::iterator aIterOnRobots;
    aIterOnRobots = _robots.begin();
    while ( aIterOnRobots != _robots.end()
            && !aIterOnRobots->second.getPlayerName().empty())
    {
        ++aIterOnRobots;
    }
    if (_robots.end() != aIterOnRobots)
    {
        aFoundRobot = aIterOnRobots->first;
    }
    return aFoundRobot;
}



}} // namespaces

