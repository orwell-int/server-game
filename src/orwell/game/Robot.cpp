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
	, m_serverCommandSocket(
		"tcp://localhost:" + boost::lexical_cast<std::string>(iServerCommandPort),
		ZMQ_REQ,
		orwell::com::ConnectionMode::CONNECT,
		m_zmqContext,
		0)
	, m_pendingImage(false)
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
	m_serverCommandSocket.sendString("capture");
	m_pendingImage = true;
}

void Robot::stop()
{
	m_serverCommandSocket.sendString("stop");
	if (not m_tempFile.empty())
	{
		// This is a bit of a hack to wait for the processes to write in the pid file
		// (this only happens when exiting very quickly, like in tests)
		size_t aSize;
		while (true)
		{
			std::ifstream aInput(m_tempFile, std::ifstream::ate | std::ifstream::binary);
			aSize = aInput.tellg();
			ORWELL_LOG_DEBUG("pid file size = " << aSize);
			if (aSize > 0)
			{
				break;
			}
			else
			{
				usleep(1000 * 50);
			}
		}
		std::ifstream aFile(m_tempFile, std::ifstream::in | std::ifstream::binary);
		int aPid = 0;
		aFile >> aPid;
		if (0 != aPid)
		{
			kill(aPid, SIGABRT);
		}
		else
		{
			ORWELL_LOG_ERROR("Could not kill a python web server ; from file " << m_tempFile);
		}
		m_tempFile.clear();
	}
}

void Robot::readImage()
{
	if (m_pendingImage)
	{
		std::string aImage;
		if (m_serverCommandSocket.receiveString(aImage, false))
		{
			ORWELL_LOG_INFO("Image received to be processed (FIRE1)");
			m_pendingImage = false;
		}
	}
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
		std::stringstream aCommandLine;
		char aTempName [] = "/tmp/video-forward.pid.XXXXXX";
		int aFileDescriptor = m_systemProxy.mkstemp(aTempName);
		if (-1 == aFileDescriptor)
		{
			ORWELL_LOG_ERROR("Unable to create temporary file (" << aTempName << ") for robot with id " << m_robotId);
			throw std::system_error(std::error_code(1, std::system_category()));
		}
		m_systemProxy.close(aFileDescriptor);

		aCommandLine << " cd server-web && make start ARGS='-u \"" <<
			m_videoUrl <<
			"\" -p " << m_videoRetransmissionPort <<
			" -l " <<  m_serverCommandPort <<
			" --pid-file " << aTempName << "'";
		ORWELL_LOG_INFO("new tmp file : " << aTempName);
		ORWELL_LOG_DEBUG("command line : " << aCommandLine.str());
		int aCode = m_systemProxy.system(aCommandLine.str().c_str());
		ORWELL_LOG_INFO("code at creation of webserver: " << aCode);

		m_tempFile = aTempName;
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
