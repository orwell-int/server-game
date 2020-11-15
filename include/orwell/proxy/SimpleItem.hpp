#pragma once

#include <memory>
#include <string>
#include <set>

#include <nlohmann/json_fwd.hpp>

namespace orwell
{

namespace game
{
class Item;
}

namespace proxy
{


struct SimpleItem
{
	SimpleItem(std::shared_ptr< game::Item const > const iItem);

	SimpleItem(SimpleItem const & iOther);

	std::string const m_name;
	std::set< std::string > m_rfids;
	int32_t m_colour;
	std::string const m_team;
};

void to_json(nlohmann::json & oJson, SimpleItem const & iItem);

}
}
