#include "orwell/game/Robot.hpp"

#include <iostream>
#include <exception>
#include <system_error>
#include <fstream>
#include <signal.h>

#include <boost/lexical_cast.hpp>

#include <zmq.hpp>

#include "orwell/com/ConnectionMode.hpp"
#include "orwell/support/GlobalLogger.hpp"
#include "orwell/support/ISystemProxy.hpp"


using namespace std;

namespace orwell
{
namespace game
{

Robot::Robot(
		support::ISystemProxy const & iSystemProxy,
		string const & iName,
		std::string const & iRobotId,
		Team & iTeam,
		uint16_t const & iVideoRetransmissionPort,
		uint16_t const & iServerCommandPort)
	: m_systemProxy(iSystemProxy)
	, m_name(iName)
	, m_robotId(iRobotId)
	, m_team(iTeam)
	, m_videoRetransmissionPort(iVideoRetransmissionPort)
	, m_serverCommandPort(iServerCommandPort)
	, m_hasRealRobot(false)
	, m_zmqContext(1) // maybe this will have to be increased for each robot ?
{
}

Robot::~Robot()
{
}

Team & Robot::getTeam()
{
	return m_team;
}

Team const & Robot::getTeam() const
{
	return m_team;
}

void Robot::setHasRealRobot(bool const iHasRealRobot)
{
	m_hasRealRobot = iHasRealRobot;
}

bool const Robot::getHasRealRobot() const
{
	return m_hasRealRobot;
}

void Robot::setPlayer(std::shared_ptr< Player > const iPlayer)
{
	m_player = iPlayer;
}

std::shared_ptr< Player > const Robot::getPlayer() const
{
	return m_player.lock();
}

bool const Robot::getHasPlayer() const
{
	return (nullptr != getPlayer().get());
}

void Robot::setVideoUrl(std::string const & iVideoUrl)
{
	m_videoUrl = iVideoUrl;
}

std::string const & Robot::getVideoUrl() const
{
	return m_videoUrl;
}

std::string const & Robot::getName() const
{
	return m_name;
}

uint16_t Robot::getVideoRetransmissionPort() const
{
	return m_videoRetransmissionPort;
}

uint16_t Robot::getServerCommandPort() const
{
	return m_serverCommandPort;
}

std::string const & Robot::getRobotId() const
{
	return m_robotId;
}

bool const Robot::getIsAvailable() const
{
	return ((m_hasRealRobot) and (not getHasPlayer()));
}

void Robot::fire()
{
}

void Robot::stop()
{
}

void Robot::readImage()
{
}

void Robot::startVideo()
{
	if (m_videoUrl.empty())
	{
		ORWELL_LOG_WARN("Robot " << m_name << " has wrong connection parameters : url='" << m_videoUrl << "'");
		return;
	}
	if (m_videoUrl.find("nc:") == 0)
	{
		ORWELL_LOG_INFO("No need to start the video from the game server with nc.");
	}
	else
	{
		ORWELL_LOG_INFO("Do not know what to do for anything else than url starting with nc:");
	}
}

//void fillRobotStateMessage( messages::RobotState & oMessage )
//{
//	//todo
//	//those are just bullshit hardcoded values to have the syntax
//	oMessage.set_life(1);
//	oMessage.mutable_move()->set_left(0);
//	oMessage.mutable_move()->set_right(0);
//	oMessage.set_active(true);
//}

}
}
