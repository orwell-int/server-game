/* This class stores most of the useful data of the server. */

#include "orwell/game/Game.hpp"

#include "orwell/game/Robot.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/com/Sender.hpp"

#include <iostream>
#include <zmq.hpp>

#include <boost/foreach.hpp>

using std::map;
using std::string;
using std::pair;
using std::shared_ptr;
using std::make_shared;

namespace orwell {
namespace game {

Game::Game()
	: _logger(log4cxx::Logger::getLogger("orwell.log"))
	, _isRunning(false)
{
}

Game::~Game()
{
}


shared_ptr<Robot> Game::accessRobot(string const & iRobotName)
{
	return _robots.at(iRobotName);
}

map<string, shared_ptr<Robot> > const & Game::getRobots()
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

bool Game::addPlayer(string const & iName)
{
	bool aAddedPlayerSuccess = false;
	if (_players.find(iName) != _players.end())
	{
		LOG4CXX_WARN(_logger, "Player name (" << iName << ") is already in the player Map.");
		aAddedPlayerSuccess = true;
	}
	else
	{
		//create playercontext and append
		Player aPlayerContext( iName );
		_players.insert(pair<string,Player>(iName, aPlayerContext));
		LOG4CXX_DEBUG(_logger, "new PlayerContext added with internalId=" << iName);
		aAddedPlayerSuccess = true;
	}

	return aAddedPlayerSuccess;
}

bool Game::addRobot(string const & iName)
{
	bool aAddedRobotSuccess = false;
	if (_robots.find(iName) != _robots.end())
	{
		LOG4CXX_WARN(_logger, "Robot name (" << iName << ") is already in the robot Map.");
	}
	else
	{
		// create RobotContext with that index
		shared_ptr<Robot> aRobot = make_shared<Robot>(iName) ;
		_robots.insert( pair<string, shared_ptr<Robot> >( iName, aRobot ) );
		LOG4CXX_DEBUG(_logger, "new RobotContext added with internal ID=" << iName);
		aAddedRobotSuccess = true;
	}
	return aAddedRobotSuccess;
}

std::shared_ptr<Robot> Game::getAvailableRobot() const
{
	shared_ptr<Robot> aFoundRobot;

	//search for the first robot which is not already associated to a player
	map<string, std::shared_ptr<Robot>>::const_iterator aIterOnRobots;
	aIterOnRobots = _robots.begin();
	while ( aIterOnRobots != _robots.end()
			&& !aIterOnRobots->second->getPlayerName().empty())
	{
		++aIterOnRobots;
	}
	
	if (_robots.end() != aIterOnRobots)
	{
		aFoundRobot = aIterOnRobots->second;
	}

	return aFoundRobot;
}

string const Game::getRobotNameForPlayer(string const & iPlayer) const
{
	string retValue;
	
	for (pair<string, std::shared_ptr<Robot>> const & iItem : _robots)
	{
		if (iItem.second.get()->getPlayerName() == iPlayer)
		{
			retValue = iItem.second.get()->getName();
		}
	}
	
	return retValue;
}
	
void Game::fillGameStateMessage(messages::GameState & oGameState)
{
	//todo
}

}} // namespaces
