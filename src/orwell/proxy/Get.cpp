#include "orwell/proxy/Get.hpp"

#include "orwell/proxy/OutputMode.hpp"
#include "orwell/proxy/Tokens.hpp"
#include "orwell/proxy/SimplePlayer.hpp"
#include "orwell/proxy/SimpleRobot.hpp"
#include "orwell/proxy/SimpleTeam.hpp"

#include "orwell/Application.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/game/Team.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orwell
{
namespace proxy
{

/// get property <property> of robot <name>
Reply GetRobot(
		OutputMode const iOutputMode,
		std::string const & iRobotName,
		std::string const & iProperty,
		orwell::game::Game const& iGame)
{
	Reply aReply;
	ORWELL_LOG_INFO("get robot '" << iRobotName << "' " << iProperty);
	try
	{
		std::shared_ptr< orwell::game::Robot const > aRobot =
			iGame.getRobot(iRobotName);
		switch (iOutputMode)
		{
			case OutputMode::kText:
			{
				if (properties::robot::kId == iProperty)
				{
					aReply = aRobot->getRobotId();
				}
				else if ("video_url" == iProperty)
				{
					aReply = aRobot->getVideoUrl();
				}
				else if ("video_port" == iProperty)
				{
					aReply = std::to_string(
							aRobot->getVideoRetransmissionPort());
				}
				else if ("video_command_port" == iProperty)
				{
					aReply = std::to_string(
							aRobot->getServerCommandPort());
				}
				else
				{
					aReply.fail(actions::reply::kKO);
					ORWELL_LOG_WARN(
							"Unknown property for a robot: '" <<
							iProperty << "'");
				}
				break;
			}

			case OutputMode::kJson:
			{
				json aJsonRobot;
				if (properties::robot::kId == iProperty)
				{
					aJsonRobot[iProperty] = aRobot->getRobotId();
				}
				// the other properties make no sense now
				else
				{
					aReply.fail();
					ORWELL_LOG_WARN(
							"Unknown property for a robot: '" <<
							iProperty << "'");
				}
				aReply = aJsonRobot.dump();
				break;
			}
		}
	}
	catch (std::exception const & aException)
	{
		ORWELL_LOG_ERROR(aException.what());
	}
	if (aReply)
	{
		ORWELL_LOG_INFO(iProperty << " = " << aReply);
	}
	return aReply;
}

/// get property <property> of team <name>
Reply GetTeam(
		OutputMode const iOutputMode,
		std::string const & iTeamName,
		std::string const & iProperty,
		orwell::game::Game const& iGame)
{
	Reply aReply;
	ORWELL_LOG_INFO("get team " << iTeamName << " " << iProperty);
	try
	{
		orwell::game::Team aTeam = iGame.getTeam(iTeamName);
		if (orwell::game::Team::GetNeutralTeam() == aTeam)
		{
			aReply.fail();
			ORWELL_LOG_WARN("Invalid team name '" << iTeamName << "'");
			return aReply;
		}
		switch (iOutputMode)
		{
			case OutputMode::kText:
			{
				if (properties::team::kScore == iProperty)
				{
					aReply = std::to_string(aTeam.getScore());
				}
				else
				{
					aReply.fail(actions::reply::kKO);
					ORWELL_LOG_WARN("Unknown property for a team: '" << iProperty << "'");
				}
				break;
			}

			case OutputMode::kJson:
			{
				json aJsonTeam;
				if (properties::team::kScore == iProperty)
				{
					aJsonTeam[iProperty] = aTeam.getScore();
				}
				else
				{
					aReply.fail();
					ORWELL_LOG_WARN("Unknown property for a team: '" << iProperty << "'");
				}
				aReply = aJsonTeam.dump();
				break;
			}
		}
	}
	catch (std::exception const & aException)
	{
		ORWELL_LOG_ERROR(aException.what());
	}
	if (aReply)
	{
		ORWELL_LOG_INFO(iProperty << " = " << aReply);
	}
	return aReply;
}

/// get property <property> of game
Reply GetGame(
		OutputMode const iOutputMode,
		std::string const & iProperty,
		orwell::game::Game const& iGame)
{
	Reply aReply;
	ORWELL_LOG_INFO("get game " << iProperty);
	try
	{
		switch (iOutputMode)
		{
			case OutputMode::kText:
			{
				if (properties::game::kTime == iProperty)
				{
					aReply = std::to_string(iGame.getSecondsLeft());
				}
				else if (properties::game::kRunning == iProperty)
				{
					aReply = std::to_string(iGame.getIsRunning());
				}
				else if (properties::game::kDuration == iProperty)
				{
					aReply = std::to_string(iGame.getDuration().total_seconds());
				}
				else
				{
					aReply.fail(actions::reply::kKO);
					ORWELL_LOG_WARN(
							"Unknown property for a game: '" << iProperty << "'");
				}
				break;
			}

			case OutputMode::kJson:
			{
				json aJsonGame;
				if (properties::game::kTime == iProperty)
				{
					aJsonGame[iProperty] = iGame.getSecondsLeft();
				}
				else if (properties::game::kRunning == iProperty)
				{
					aJsonGame[iProperty] = iGame.getIsRunning();
				}
				else if (properties::game::kDuration == iProperty)
				{
					aJsonGame[iProperty] = iGame.getDuration().total_seconds();
				}
				else
				{
					aReply.fail();
					ORWELL_LOG_WARN(
							"Unknown property for a game: '" << iProperty << "'");
				}
				aReply = aJsonGame.dump();
				break;
			}
		}
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
	if (aReply)
	{
		ORWELL_LOG_INFO(iProperty << " = " << aReply);
	}
	return aReply;
}

Get::Get()
	: IAction("get")
{
}

Reply Get::process(
		OutputMode const iOutputMode,
		std::istringstream & ioStream,
		orwell::Application & ioApplication,
		orwell::game::Game const& iGame) const
{
	Reply aReply;
	ORWELL_LOG_DEBUG("Get::process");
	std::string aObject;
	ioStream >> aObject;
	bool const aIsRobot = (objects::kRobot == aObject);
	bool const aIsTeam = (objects::kTeam == aObject);
	std::string aName;
	if ((aIsRobot) or (aIsTeam))
	{
		aName = ReadName(ioStream);
	}
	std::string aProperty;
	ioStream >> aProperty;
	if (aIsRobot)
	{
		aReply = GetRobot(iOutputMode, aName, aProperty, iGame);
	}
	else if (aIsTeam)
	{
		aReply = GetTeam(iOutputMode, aName, aProperty, iGame);
	}
	else if (objects::kGame == aObject)
	{
		aReply = GetGame(iOutputMode, aProperty, iGame);
	}
	else
	{
		switch (iOutputMode)
		{
			case OutputMode::kText:
			{
				aReply.fail(
						"Unexpected text after list "
						+ aObject + ": '" + ioStream.str() + "'");
				break;
			}
			case OutputMode::kJson:
			{
				aReply.fail();
				break;
			}
		}
	}
	return aReply;
}

}
}
