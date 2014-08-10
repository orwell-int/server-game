/* This class stores most of the useful data of the server. */

#include "orwell/game/Game.hpp"

#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <signal.h>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/Server.hpp"

#include <iostream>
#include <zmq.hpp>

#include <boost/lexical_cast.hpp>

using std::map;
using std::string;
using std::pair;
using std::shared_ptr;
using std::make_shared;

namespace orwell {
namespace game {

Game::Game(
		boost::posix_time::time_duration const & iGameDuration,
		Server & ioServer)
	: m_isRunning(false)
	, m_gameDuration(iGameDuration)
	, m_server(ioServer)
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
		ORWELL_LOG_INFO("new PlayerContext added with internalId=" << iName);
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

bool Game::getIsRunning() const
{
	return m_isRunning;
}

void Game::start()
{
	ORWELL_LOG_DEBUG( "Game::start");
	if (not m_isRunning)
	{
		for ( auto const aPair : m_robots )
		{
			std::shared_ptr< Robot > aRobot = aPair.second;
			std::stringstream aCommandLine;
			if (aRobot->getVideoUrl().empty())
			{
				ORWELL_LOG_WARN("Robot " << aRobot->getName() << " has wrong connection parameters : url=" << aRobot->getVideoUrl());
				continue;
			}
			char * aTempName = tmpnam(nullptr);
			std::ofstream(aTempName).close();

			aCommandLine << " cd server-web && make start ARGS='-u \"" <<
				aRobot->getVideoUrl() <<
				"\" -p " << aRobot->getVideoRetransmissionPort() <<
				" -l " <<  aRobot->getServerCommandPort() <<
				" --pid-file " << aTempName << "'";
			ORWELL_LOG_INFO("new tmp file : " << aTempName);
			ORWELL_LOG_DEBUG("command line : " << aCommandLine.str());
			int aCode = system(aCommandLine.str().c_str());
			ORWELL_LOG_INFO("code at creation of webserver: " << aCode);

			m_tmpFiles.push_back(aTempName);

			m_server.addServerCommandSocket(aRobot->getRobotId(), aRobot->getServerCommandPort());
		}
		ORWELL_LOG_INFO("game starts");
		m_startTime = boost::posix_time::microsec_clock::local_time();
		m_isRunning = true;
	}
}

void Game::stop()
{
	ORWELL_LOG_INFO("GAME STOP");
	if (m_isRunning)
	{
		for (auto const aPair : m_robots)
		{
			std::shared_ptr< Robot > aRobot = aPair.second;
			m_server.sendServerCommand(aRobot->getRobotId(), "stop");
		}
		for ( auto const aFileName: m_tmpFiles )
		{
			std::ifstream aFile(aFileName, std::ifstream::in);
			int aPid = 0;
			aFile >> aPid;
			if (0 != aPid)
			{
				kill(aPid, SIGABRT);
			}
			else
			{
				ORWELL_LOG_ERROR("Could not kill a python web server.");
			}
		}
		ORWELL_LOG_INFO( "game stops" );
		m_isRunning = false;
	}
}

bool Game::addRobot(
		string const & iName,
		uint16_t const iVideoRetransmissionPort,
		uint16_t const iServerCommandPort,
		std::string iRobotId)
{
	bool aAddedRobotSuccess = false;
	if (m_robots.find(iName) != m_robots.end())
	{
		ORWELL_LOG_WARN("Robot name (" << iName << ") is already in the robot Map.");
	}
	else
	{
		// create Robot with that index
		if (iRobotId.empty())
		{
			iRobotId = getNewRobotId();
		}
		shared_ptr<Robot> aRobot = make_shared<Robot>(iName, iRobotId, iVideoRetransmissionPort, iServerCommandPort);
		m_robots.insert( pair<string, shared_ptr<Robot> >( iName, aRobot ) );
		m_robotsById.insert(pair< string, shared_ptr< Robot > >(iRobotId, aRobot));
		ORWELL_LOG_INFO("new Robot added with name='" << iName << "', " <<
			"ID='" << iRobotId << "'");
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

void Game::fire(std::string const & iRobotId)
{
	ORWELL_LOG_DEBUG("Try fire from robot: " << iRobotId);
	if (m_robotsById.end() != m_robotsById.find(iRobotId))
	{
		m_server.sendServerCommand(iRobotId, "capture");
		m_pendingImage.insert(iRobotId);
	}
	else
	{
		ORWELL_LOG_INFO("Try to fire from missing robot: " << iRobotId);
	}
}

void Game::readImages()
{
	std::set< std::string >::iterator aPending = m_pendingImage.begin();
	while (m_pendingImage.end() != aPending)
	{
		std::set< std::string >::iterator aCurrent = aPending++;
		std::string const & aRobotId = *aCurrent;
		std::string aImage;
		if (m_server.receiveCommandResponse(aRobotId, aImage))
		{
			m_pendingImage.erase(aCurrent);
			ORWELL_LOG_INFO("Image received to be processed (FIRE1)");
		}
	}
}

std::shared_ptr< Robot > Game::getRobotWithoutRealRobot(
		std::string const & iTemporaryRobotId) const
{
	shared_ptr< Robot > aFoundRobot;
	auto const aRegistrationIterator = m_registeredRobots.find(iTemporaryRobotId);
	if (m_registeredRobots.end() != aRegistrationIterator)
	{
		aFoundRobot = m_robots.at(aRegistrationIterator->second);
	}
	else
	{
		map< string, std::shared_ptr< Robot > >::const_iterator aIterOnRobots;
		aIterOnRobots = m_robots.begin();
		while (aIterOnRobots != m_robots.end() && aIterOnRobots->second->getHasRealRobot())
		{
			++aIterOnRobots;
		}

		if (m_robots.end() != aIterOnRobots)
		{
			aFoundRobot = aIterOnRobots->second;
			m_registeredRobots[iTemporaryRobotId] = aIterOnRobots->first;
		}
	}

	return aFoundRobot;
}

std::shared_ptr<Robot> Game::getAvailableRobot() const
{
	shared_ptr<Robot> aFoundRobot;

	//search for the first robot which is not already associated to a player
	map<string, std::shared_ptr<Robot>>::const_iterator aIterOnRobots;
	aIterOnRobots = m_robots.begin();
	while (aIterOnRobots != m_robots.end() && (not aIterOnRobots->second->getIsAvailable()))
	{
		++aIterOnRobots;
	}

	if (m_robots.end() != aIterOnRobots)
	{
		aFoundRobot = aIterOnRobots->second;
	}

	return aFoundRobot;
}

std::shared_ptr< Robot > Game::getRobotForPlayer(string const & iPlayer) const
{
	std::shared_ptr< Robot > aFoundRobot;
	
	for (pair<string, std::shared_ptr<Robot>> const & iItem : m_robots)
	{
		std::shared_ptr< Player > aPlayer = iItem.second.get()->getPlayer();
		if ((nullptr != aPlayer) and (aPlayer->getName() == iPlayer))
		{
			aFoundRobot = iItem.second;
		}
	}
	if (nullptr == aFoundRobot.get())
	{
		aFoundRobot = getAvailableRobot();
	}
	return aFoundRobot;
}
	
void Game::fillGameStateMessage(messages::GameState & oGameState)
{
	//todo
}

void Game::setTime(boost::posix_time::ptime const & iCurrentTime)
{
	m_time = iCurrentTime;
}

void Game::stopIfGameIsFinished()
{
	if (m_gameDuration <= m_startTime - m_time)
	{
		stop();
	}
}

std::string Game::getNewRobotId() const
{
	std::string const aRobotIdPrefix("robot_");
	std::string aFullRobotId;
	uint32_t aIndex = 0;
	bool aAlreadyThere(true);
	while (aAlreadyThere)
	{
		aAlreadyThere = false;
		aFullRobotId = aRobotIdPrefix + boost::lexical_cast< std::string >(aIndex);
		for (std::pair< std::string, std::shared_ptr< Robot > > const & iItem : m_robots)
		{
			if (iItem.second->getRobotId() == aFullRobotId)
			{
				aAlreadyThere = true;
				break;
			}
		}
		++aIndex;
	}
	return aFullRobotId;
}


}
} // namespaces

