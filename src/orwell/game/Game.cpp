// This class stores most of the useful data of the server.

#include "orwell/game/Game.hpp"

#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <utility>
#include <exception>
#include <system_error>

#include <boost/lexical_cast.hpp>

#include <zmq.hpp>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/support/ISystemProxy.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/game/Contact.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/Server.hpp"
#include "orwell/game/Ruleset.hpp"
#include "orwell/game/item/FlagDetector.hpp"

#include "MissingFromTheStandard.hpp"

namespace orwell
{
namespace game
{

Game::Game(
		support::ISystemProxy const & iSystemProxy,
		boost::posix_time::time_duration const & iGameDuration,
		Ruleset const & iRuleset)
	: m_systemProxy(iSystemProxy)
	, m_isRunning(false)
	, m_gameDuration(iGameDuration)
	, m_ruleset(iRuleset)
{
	ORWELL_LOG_DEBUG("Game duration: " << m_gameDuration.total_seconds() << " second(s).");
}

Game::~Game()
{
}


std::shared_ptr<Robot> Game::accessRobot(std::string const & iRobotName)
{
	return m_robots.at(iRobotName);
}

std::shared_ptr<Robot const> Game::getRobot(std::string const & iRobotName) const
{
	return m_robots.at(iRobotName);
}

std::shared_ptr< Robot > Game::accessRobotById(std::string const & iRobotName)
{
	return m_robotsById.at(iRobotName);
}

bool Game::getHasRobotById(std::string const & iRobotId) const
{
	return (m_robotsById.end() != m_robotsById.find(iRobotId));
}

std::map<std::string, std::shared_ptr<Robot> > const & Game::getRobots() const
{
	return m_robots;
}

std::shared_ptr< Player > Game::accessPlayer(std::string const & iPlayerName)
{
	return m_players.at(iPlayerName);
}

std::shared_ptr< Player const > Game::getPlayer(std::string const & iPlayerName) const
{
	return m_players.at(iPlayerName);
}

std::map< std::string, std::shared_ptr< Player > > const & Game::getPlayers() const
{
	return m_players;
}

bool Game::addPlayer(std::string const & iName)
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

bool Game::removePlayer(std::string const & iName)
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

uint64_t Game::getSecondsLeft() const
{
	if (m_isRunning)
	{
		boost::posix_time::time_duration aEllapsed = m_time - m_startTime;
		return (m_gameDuration.total_seconds() - aEllapsed.total_seconds());
	}
	else
	{
		return m_gameDuration.total_seconds();
	}
}

boost::posix_time::time_duration const & Game::getDuration() const
{
	return m_gameDuration;
}

void Game::setDuration(boost::posix_time::time_duration const & iSeconds)
{
	m_gameDuration = iSeconds;
}

void Game::start(
		bool const iForceStop,
		boost::optional< boost::posix_time::ptime > const iForcedStartTime)
{
	ORWELL_LOG_DEBUG("Game::start");
	if (iForceStop)
	{
		m_winner.reset();
		for (auto & aPair : m_teams)
		{
			aPair.second.resetScore();
		}
		stop();
	}
	if (not m_isRunning)
	{
		for (auto const & aPair : m_robots)
		{
			std::shared_ptr< Robot > aRobot = aPair.second;
			try
			{
				aRobot->startVideo();
			}
			catch (std::system_error const & aError)
			{
				ORWELL_LOG_ERROR("Could not start the video. Abort.");
				m_isRunning = true;
				stop();
				abort();
			}
		}
		ORWELL_LOG_INFO("game starts");
		if (iForcedStartTime)
		{
			m_startTime = *iForcedStartTime;
		}
		else
		{
			m_startTime = boost::posix_time::microsec_clock::local_time();
		}
		m_time = m_startTime;
		m_isRunning = true;
	}
}

void Game::stop()
{
	ORWELL_LOG_INFO("GAME STOP");
	if (m_isRunning)
	{
		for (auto const & aPair : m_robots)
		{
			std::shared_ptr< Robot > aRobot = aPair.second;
			aRobot->stop();
		}
		ORWELL_LOG_INFO("game stops");
		m_isRunning = false;
		orwell::game::Item::ResetAllItems();
	}
}

bool Game::addTeam(std::string const & iTeamName)
{
	bool aAdded(false);
	if (m_teams.end() == m_teams.find(iTeamName))
	{
		m_teams[iTeamName] = Team(iTeamName);
		//m_teams.insert(std::make_pair< std::string, Team >(iTeamName, Team(iTeamName)));
		aAdded = true;
	}
	return aAdded;
}

bool Game::removeTeam(std::string const & iTeamName)
{
	bool aRemoved(false);
	auto aFound = m_teams.find(iTeamName);
	if (m_teams.end() != aFound)
	{
		m_teams.erase(aFound);
		aRemoved = true;
	}
	return aRemoved;
}

void Game::getTeams(std::vector< std::string > & ioTeams) const
{
	for (auto const & aPair : m_teams)
	{
		ioTeams.push_back(aPair.first);
	}
}

Team const & Game::getTeam(std::string const & iTeamName) const
{
	auto aFound = m_teams.find(iTeamName);
	return (m_teams.end() != aFound)
		? aFound->second
		: Team::GetNeutralTeam();
}

Team & Game::accessTeam(std::string const & iTeamName)
{
	auto aFound = m_teams.find(iTeamName);
	return (m_teams.end() != aFound)
		? aFound->second
		: Team::GetNeutralTeam();
}

bool Game::addRobot(
		std::string const & iName,
		std::string const & iTeamName,
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
		std::map<std::string, Team>::iterator aTeamIterator = m_teams.find(iTeamName);
		if (m_teams.end() != aTeamIterator)
		{
			std::shared_ptr<Robot> aRobot = Robot::MakeRobot(
					m_systemProxy,
					iName,
					iRobotId,
					aTeamIterator->second,
					iVideoRetransmissionPort,
					iServerCommandPort);
			m_robots.insert(std::make_pair<>(iName, aRobot));
			m_robotsById.insert(std::make_pair<>(iRobotId, aRobot));
			ORWELL_LOG_INFO("new Robot added with name='" << iName << "', " <<
					"ID='" << iRobotId << "'");
			std::shared_ptr< item::FlagDetector > aFlagDetector =
				std::make_shared< item::FlagDetector >(*this, aRobot);
			m_flagDetectorsByRobot.insert(std::make_pair<>(iRobotId, aFlagDetector));
			aAddedRobotSuccess = true;
		}
	}
	return aAddedRobotSuccess;
}

bool Game::removeRobot(std::string const & iName)
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

void Game::step()
{
	readImages();
	handleContacts();
	stopIfGameIsFinished();
}

std::shared_ptr< Robot > Game::getRobotWithoutRealRobot(
		std::string const & iTemporaryRobotId) const
{
	std::shared_ptr< Robot > aFoundRobot;
	auto const aRegistrationIterator = m_registeredRobots.find(iTemporaryRobotId);
	if (m_registeredRobots.end() != aRegistrationIterator)
	{
		aFoundRobot = m_robots.at(aRegistrationIterator->second);
	}
	else
	{
		std::map< std::string, std::shared_ptr< Robot > >::const_iterator aIterOnRobots;
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
	std::shared_ptr<Robot> aFoundRobot;

	//search for the first robot which is not already associated to a player
	std::map<std::string, std::shared_ptr<Robot>>::const_iterator aIterOnRobots;
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

boost::optional< std::string > const & Game::getWinner() const
{
	return m_winner;
}

std::shared_ptr< Robot > Game::getRobotForPlayer(std::string const & iPlayer) const
{
	std::shared_ptr< Robot > aFoundRobot;
	
	for (auto const & aElemement : m_robots)
	{
		std::shared_ptr< Player > aPlayer = aElemement.second.get()->getPlayer();
		if ((nullptr != aPlayer) and (aPlayer->getName() == iPlayer))
		{
			aFoundRobot = aElemement.second;
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

boost::posix_time::ptime const & Game::getTime() const
{
	return m_time;
}

void Game::stopIfGameIsFinished()
{
	uint64_t aSecondsLeft(getSecondsLeft());
	if (aSecondsLeft <= 0)
	{
		ORWELL_LOG_INFO("stop ; excessive time spent " << aSecondsLeft);
		stop();
	}
	else
	{
		std::vector< std::string > aWinningTeams;
		for (auto const & aTeamElement : m_teams)
		{
			if (aTeamElement.second.getScore() >= m_ruleset.m_scoreToWin)
			{
				aWinningTeams.push_back(aTeamElement.second.getName());
			}
		}
		if (aWinningTeams.empty())
		{
			// nobody has won yet
		}
		else if (1 == aWinningTeams.size())
		{
			ORWELL_LOG_INFO("stop ; we have a winner");
			m_winner = aWinningTeams.front();
			stop();
		}
		else
		{
			ORWELL_LOG_INFO("stop ; stalemate");
			//todo
			stop();
		}
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
		aFullRobotId = aRobotIdPrefix + std::to_string(aIndex);
		for (auto const & aElemement : m_robots)
		{
			if (aElemement.second->getRobotId() == aFullRobotId)
			{
				aAlreadyThere = true;
				break;
			}
		}
		++aIndex;
	}
	return aFullRobotId;
}

void Game::readImages()
{
	for (auto const & aElemement : m_robots)
	{
		aElemement.second->readImage();
	}
}

void Game::handleContacts()
{
	ContactMap::const_iterator aIter = m_contacts.begin();
	while (m_contacts.end() != aIter)
	{
		auto const & aContactPair = *aIter;
		StepSignal const aSignal = aContactPair.second->step(m_time);
		switch (aSignal)
		{
			case StepSignal::SILENCEIKILLU:
			{
				aIter = m_contacts.erase(aIter);
				break;
			}
			default:
			{
				++aIter;
			}
		}
	}
}

void Game::robotIsInContactWith(std::string const & iRobotId, std::shared_ptr<Item> const iItem)
{
	ORWELL_LOG_INFO("Robot " << iRobotId << " records contact with item " << iItem);
	// here we suppose that a robot can only be in contact with one item.
	m_contacts[iRobotId] = make_unique<Contact>(
			m_time,
			m_ruleset.m_timeToCapture,
			m_robotsById[iRobotId],
			iItem);
}

void Game::robotDropsContactWith(std::string const & iRobotId, std::shared_ptr<Item> const iItem)
{
	ORWELL_LOG_INFO("Robot " << iRobotId << " drops contact with item " << iItem);
	m_contacts.erase(iRobotId);
}

void Game::setMapLimits(std::vector< orwell::game::Landmark > const & iMapLimits)
{
	m_mapLimits = iMapLimits;
}

std::vector< orwell::game::Landmark > const & Game::getMapLimits() const
{
	return m_mapLimits;
}

std::weak_ptr< orwell::game::item::FlagDetector > Game::getFlagDetector(std::string const & iRobotId)
{
	std::weak_ptr< orwell::game::item::FlagDetector > aResult;
	auto const aFound = m_flagDetectorsByRobot.find(iRobotId);
	if (m_flagDetectorsByRobot.end() != aFound)
	{
		aResult = aFound->second;
	}
	return aResult;
}

std::string Game::getAsString() const
{
	std::string aReply = "Game ";
	aReply += "time = " + std::to_string(getSecondsLeft()) + " ; ";
	aReply += "running = " + std::to_string(m_isRunning) + " ; ";
	aReply += "duration = " + std::to_string(m_gameDuration.total_seconds());
	return aReply;
}

} // namespace game
} // namespace orwell
