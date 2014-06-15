#include "orwell/game/Robot.hpp"
#include <iostream>

#include "robot.pb.h"

using namespace std;
using orwell::messages::RobotState;

namespace orwell {
namespace game {

Robot::Robot(
		string const & iName,
		std::string const & iRobotId)
	: m_name(iName)
	, m_robotId(iRobotId)
	, m_hasRealRobot(false)
{
}

Robot::~Robot()
{
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

string const & Robot::getName() const
{
	return m_name;
}

std::string const & Robot::getRobotId() const
{
	return m_robotId;
}

bool const Robot::getIsAvailable() const
{
	return ((m_hasRealRobot) and (not getHasPlayer()));
}

void fillRobotStateMessage( messages::RobotState & oMessage )
{
	//todo
	//those are just bullshit hardcoded values to have the syntax
	oMessage.set_life(1);
	oMessage.mutable_move()->set_left(0);
	oMessage.mutable_move()->set_right(0);
	oMessage.set_active(true);
}

}
} // namespaces

