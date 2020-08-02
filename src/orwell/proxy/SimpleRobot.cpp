#include "orwell/proxy/SimpleRobot.hpp"

#include "orwell/game/Robot.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/game/Team.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orwell
{
namespace proxy
{

void to_json(json & oJson, SimpleRobot const & iRobot)
{
	oJson = json {
			{"name", iRobot.m_name},
			{"player", iRobot.m_player},
			{"registered", iRobot.m_registered},
			{"team", iRobot.m_team},
			{"video_url", iRobot.m_videoUrl},
	};
}


SimpleRobot::SimpleRobot(std::shared_ptr< game::Robot const > const iRobot)
	: m_name(iRobot->getName())
	, m_player(iRobot->getHasPlayer() ? iRobot->getPlayer()->getName() : "")
	, m_registered(iRobot->getHasRealRobot())
	, m_team(iRobot->getTeam().getName())
	, m_videoUrl(iRobot->getVideoUrl())
{
}

SimpleRobot::SimpleRobot(SimpleRobot const & iOther)
	: m_name(iOther.m_name)
	, m_player(iOther.m_player)
	, m_registered(iOther.m_registered)
	, m_team(iOther.m_team)
	, m_videoUrl(iOther.m_videoUrl)
{
}

}
}
