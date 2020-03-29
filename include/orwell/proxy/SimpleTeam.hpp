#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace orwell
{

namespace game
{
class Team;
}

namespace proxy
{

struct SimpleTeam
{
	SimpleTeam(game::Team const & iTeam);

	SimpleTeam(SimpleTeam const & iOther);

	std::string const m_name;
	uint32_t const m_score;
	std::vector< std::string > const m_robots;
};

void to_json(nlohmann::json & oJson, SimpleTeam const & iTeam);

}
}
