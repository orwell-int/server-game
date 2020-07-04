#include "orwell/proxy/View.hpp"

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

/// view a team in details
Reply ViewTeam(
		OutputMode const iOutputMode,
		std::string const & iName,
		orwell::game::Game const& iGame)
{
	Reply aReply;
	ORWELL_LOG_INFO("view team '" << iName << "' " << iOutputMode);
	game::Team const & aTeam = iGame.getTeam(iName);
	switch (iOutputMode)
	{
		case OutputMode::kText:
		{
			if (aTeam.getIsNeutralTeam())
			{
				aReply.fail("Invalid team name (" + iName + ")");
				break;
			}
			aReply = "Team " + iName + ":\n";
			aReply += "\tscore = " + std::to_string(aTeam.getScore())  + " ; ";
			aReply += "robots = [";

			bool aFirst(true);
			for (std::string const & aRobotName: aTeam.getRobots())
			{
				if (aFirst)
				{
					aFirst = false;
				}
				else
				{
					aReply += ", ";
				}
				aReply += "\"" + aRobotName + "\"";
			}
			aReply += "]";
			break;
		}
		case OutputMode::kJson:
		{
			json aJsonTeam;
			if (aTeam.getIsNeutralTeam())
			{
				aJsonTeam[objects::reply::kTeam] = nullptr;
			}
			else
			{
				aJsonTeam[objects::reply::kTeam] = SimpleTeam(aTeam);
			}
			aReply = aJsonTeam.dump();
			break;
		}
	}
	return aReply;
}

View::View()
	: IAction("view")
{
}

Reply View::process(
		OutputMode const iOutputMode,
		std::istringstream & ioStream,
		orwell::Application & ioApplication,
		orwell::game::Game const& iGame) const
{
	ORWELL_LOG_DEBUG("View::process");
	Reply aReply;
	std::string aObject;
	ioStream >> aObject;
	if (objects::kTeam == aObject)
	{
		std::string const aName = ReadName(ioStream);
		if (ioStream.eof())
		{
			aReply = ViewTeam(iOutputMode, aName, iGame);
		}
		else
		{
			aReply.fail(
					"Unexpected text after view "
					+ aObject + " " + aName + ": '" + ioStream.str() + "'");
		}
	}
	else
	{
		aReply.fail("Invalid object for view: '" + aObject + "'");
	}
	return aReply;
}

}
}
