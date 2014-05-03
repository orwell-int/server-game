/* This class stores most of the useful data of the server. */

#include "orwell/game/Game.hpp"

#include "orwell/support/GlobalLogger.hpp"
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
	: m_isRunning(false)
{
}

Game::~Game()
{
}


shared_ptr<Robot> Game::accessRobot(string const & iRobotName)
{
	return m_robots.at(iRobotName);
}

map<string, shared_ptr<Robot> > const & Game::getRobots()
{
	return m_robots;
}

std::shared_ptr< Player > Game::accessPlayer( string const & iPlayerName)
{
	return m_players.at(iPlayerName);
}

map< string, std::shared_ptr< Player > > const & Game::getPlayers()
{
	return m_players;
}

bool Game::addPlayer(string const & iName)
{
	bool aAddedPlayerSuccess = false;
	if (m_players.find(iName) != m_players.end())
	{
		ORWELL_LOG_WARN("Player name (" << iName << ") is already in the player Map.");
		aAddedPlayerSuccess = true;
	}
	else
	{
		//create playercontext and append
		std::shared_ptr< Player > aPlayer = std::make_shared< Player >(iName);
		m_players.insert(std::make_pair(iName, aPlayer));
		ORWELL_LOG_DEBUG("new PlayerContext added with internalId=" << iName);
		aAddedPlayerSuccess = true;
	}

	return aAddedPlayerSuccess;
}

bool Game::removePlayer(string const & iName)
{
	bool aRemovedPlayerSuccess = false;
	auto aFound = m_players.find(iName);
	if (aFound != m_players.end())
	{
		m_players.erase(aFound);
		aRemovedPlayerSuccess = true;
	}
	return aRemovedPlayerSuccess;
}

bool Game::addRobot(string const & iName)
{
	bool aAddedRobotSuccess = false;
	if (m_robots.find(iName) != m_robots.end())
	{
		ORWELL_LOG_WARN("Robot name (" << iName << ") is already in the robot Map.");
	}
	else
	{
		// create RobotContext with that index
		shared_ptr<Robot> aRobot = make_shared<Robot>(iName) ;
		m_robots.insert( pair<string, shared_ptr<Robot> >( iName, aRobot ) );
		ORWELL_LOG_DEBUG("new RobotContext added with internal ID=" << iName);
		aAddedRobotSuccess = true;
	}
	return aAddedRobotSuccess;
}

bool Game::removeRobot(string const & iName)
{
	bool aRemovedRobotSuccess = false;
	auto aFound = m_robots.find(iName);
	if (aFound != m_robots.end())
	{
		m_robots.erase(aFound);
		aRemovedRobotSuccess = true;
	}
	return aRemovedRobotSuccess;
}

std::shared_ptr<Robot> Game::getAvailableRobot() const
{
	shared_ptr<Robot> aFoundRobot;

	//search for the first robot which is not already associated to a player
	map<string, std::shared_ptr<Robot>>::const_iterator aIterOnRobots;
	aIterOnRobots = m_robots.begin();
	while (aIterOnRobots != m_robots.end() && aIterOnRobots->second->getPlayer())
	{
		++aIterOnRobots;
	}
	
	if (m_robots.end() != aIterOnRobots)
	{
		aFoundRobot = aIterOnRobots->second;
	}

	return aFoundRobot;
}

string const Game::getRobotNameForPlayer(string const & iPlayer) const
{
	string retValue;
	
	for (pair<string, std::shared_ptr<Robot>> const & iItem : m_robots)
	{
		std::shared_ptr< Player > aPlayer = iItem.second.get()->getPlayer();
		if ((nullptr != aPlayer) and (aPlayer->getName() == iPlayer))
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

