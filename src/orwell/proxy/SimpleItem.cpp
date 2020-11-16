#include "orwell/proxy/SimpleItem.hpp"

#include "orwell/game/Item.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orwell
{
namespace proxy
{

void to_json(json & oJson, SimpleItem const & iItem)
{
	oJson = json {
			{"name", iItem.m_name},
			{"RFID", iItem.m_rfids},
			{"colour", iItem.m_colour},
			{"team", iItem.m_team},
	};
}


SimpleItem::SimpleItem(std::shared_ptr< game::Item const > const iItem)
	: m_name(iItem->getName())
	, m_rfids(iItem->getRfids())
	, m_colour(iItem->getColour())
	, m_team(iItem->getTeam())
{
}

SimpleItem::SimpleItem(SimpleItem const & iOther)
	: m_name(iOther.m_name)
	, m_rfids(iOther.m_rfids)
	, m_colour(iOther.m_colour)
	, m_team(iOther.m_team)
{
}

}
}
