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

#include <boost/optional/optional_io.hpp>

using json = nlohmann::json;

namespace orwell
{
namespace proxy
{

/// get property <property> of robot <name>
Reply GetRobot(
		OutputMode const iOutputMode,
		std::string const & iRobotName,
		boost::optional< std::string > const & iProperty,
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
				if (iProperty)
				{
					if (properties::robot::kId == *iProperty)
					{
						aReply = aRobot->getRobotId();
					}
					else if (properties::robot::kVideoUrl == *iProperty)
					{
						aReply = aRobot->getVideoUrl();
					}
					else if (properties::robot::kVideoPort == *iProperty)
					{
						aReply = std::to_string(
								aRobot->getVideoRetransmissionPort());
					}
					else if (properties::robot::kVideoCommandPort == *iProperty)
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
				}
				else
				{
					aReply = aRobot->getAsString();
				}
				break;
			}

			case OutputMode::kJson:
			{
				json aJsonRobot;
				if (iProperty)
				{
					if (properties::robot::kId == *iProperty)
					{
						aJsonRobot[*iProperty] = aRobot->getRobotId();
					}
					// the other properties make no sense now
					else
					{
						aReply.fail();
						ORWELL_LOG_WARN(
								"Unknown property for a robot: '" <<
								iProperty << "'");
					}
				}
				else
				{
					to_json(aJsonRobot, aRobot);
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
		boost::optional< std::string > const & iProperty,
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
				if (not iProperty)
				{
					aReply.fail(actions::reply::kKO);
					ORWELL_LOG_WARN("Missing property for get team");
				}
				else if (properties::team::kScore == *iProperty)
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
				if (not iProperty)
				{
					aReply.fail();
					ORWELL_LOG_WARN("Missing property for get team");
				}
				else if (properties::team::kScore == *iProperty)
				{
					aJsonTeam[*iProperty] = aTeam.getScore();
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

/// get property <property> of team <name>
Reply GetPlayer(
		OutputMode const iOutputMode,
		std::string const & iPlayerName,
		boost::optional< std::string > const & iProperty,
		orwell::game::Game const& iGame)
{
	Reply aReply;
	ORWELL_LOG_INFO("get player " << iPlayerName << " " << iProperty);
	try
	{
		std::shared_ptr< orwell::game::Player const > aPlayer = iGame.getPlayer(iPlayerName);
		switch (iOutputMode)
		{
			case OutputMode::kText:
			{
				if (iProperty)
				{
					if (properties::player::kAddress == *iProperty)
					{
						aReply = aPlayer->getAddress();
					}
					else
					{
						aReply.fail(actions::reply::kKO);
						ORWELL_LOG_WARN("Unknown property for a team: '" << *iProperty << "'");
					}
				}
				else
				{
					aReply = aPlayer->getAsString();
				}
				break;
			}

			case OutputMode::kJson:
			{
				json aJsonPlayer;
				if (iProperty)
				{
					if (properties::player::kAddress == *iProperty)
					{
						aJsonPlayer[*iProperty] = aPlayer->getAddress();
					}
					else
					{
						aReply.fail();
						ORWELL_LOG_WARN("Unknown property for a team: '" << *iProperty << "'");
					}
				}
				else
				{
					to_json(aJsonPlayer, aPlayer);
				}
				aReply = aJsonPlayer.dump();
				break;
			}
		}
	}
	catch (std::out_of_range const & aException)
	{
		aReply.fail();
		ORWELL_LOG_WARN("Invalid player name '" << iPlayerName << "'");
		return aReply;
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


/// get all properties of game
Reply GetGameJson(orwell::game::Game const& iGame)
{
	Reply aReply;
	ORWELL_LOG_INFO("get game");
	json aJsonGame;
	aJsonGame[properties::game::kTime] = iGame.getSecondsLeft();
	aJsonGame[properties::game::kRunning] = iGame.getIsRunning();
	aJsonGame[properties::game::kDuration] = iGame.getDuration().total_seconds();
	aReply = aJsonGame.dump();
	return aReply;
}

/// get property <property> of game
Reply GetGameText(
		boost::optional< std::string > const & iProperty,
		orwell::game::Game const& iGame)
{
	Reply aReply;
	ORWELL_LOG_INFO("get game " << iProperty);
	try
	{
		if (iProperty)
		{
			if (properties::game::kTime == *iProperty)
			{
				aReply = std::to_string(iGame.getSecondsLeft());
			}
			else if (properties::game::kRunning == *iProperty)
			{
				aReply = std::to_string(iGame.getIsRunning());
			}
			else if (properties::game::kDuration == *iProperty)
			{
				aReply = std::to_string(iGame.getDuration().total_seconds());
			}
			else
			{
				aReply.fail(actions::reply::kKO);
				ORWELL_LOG_WARN(
						"Unknown property for a game: '" << iProperty << "'");
			}
		}
		else
		{
			aReply = iGame.getAsString();
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
	bool const aIsGame = (objects::kGame == aObject);
	bool const aIsPlayer = (objects::kPlayer == aObject);
	std::string aName;
	if ((aIsRobot) or (aIsTeam) or (aIsPlayer))
	{
		aName = ReadName(ioStream);
	}
	if ((aIsGame) and (OutputMode::kJson == iOutputMode))
	{
		aReply = GetGameJson(iGame);
	}
	else
	{
		boost::optional< std::string > aProperty;
		if (not ioStream.eof())
		{
			std::string aString;
			ioStream >> aString;
			aProperty = aString;
		}
		if (aIsRobot)
		{
			aReply = GetRobot(iOutputMode, aName, aProperty, iGame);
		}
		else if (aIsTeam)
		{
			aReply = GetTeam(iOutputMode, aName, aProperty, iGame);
		}
		else if (aIsGame)
		{
			aReply = GetGameText(aProperty, iGame);
		}
		else if (aIsPlayer)
		{
			aReply = GetPlayer(iOutputMode, aName, aProperty, iGame);
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
	}
	return aReply;
}

}
}
