#include "orwell/proxy/SimpleTeam.hpp"

#include "orwell/game/Team.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/game/Player.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orwell
{

namespace game
{
class Player;
}

namespace proxy
{

void to_json(json & oJson, SimpleTeam const & iTeam)
{
	oJson = json {
			{"name", iTeam.m_name},
			{"score", iTeam.m_score},
			{"robots", iTeam.m_robots}
	};
}


SimpleTeam::SimpleTeam(game::Team const & iTeam)
	: m_name(iTeam.getName())
	, m_score(iTeam.getScore())
	, m_robots(iTeam.getRobots())
{
}

SimpleTeam::SimpleTeam(SimpleTeam const & iOther)
	: m_name(iOther.m_name)
	, m_score(iOther.m_score)
	, m_robots(iOther.m_robots)
{
}

}
}
