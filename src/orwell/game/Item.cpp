#include "orwell/game/Item.hpp"

#include "orwell/game/Team.hpp"
#include "orwell/game/item/Flag.hpp"
#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Ruleset.hpp"

#include <sstream>

std::map< std::string, std::shared_ptr< orwell::game::Item > > orwell::game::Item::s_itemsByRfid = std::map<std::string, std::shared_ptr<orwell::game::Item> >();
std::map< int32_t, std::shared_ptr< orwell::game::Item > > orwell::game::Item::s_itemsByColour = std::map<int32_t, std::shared_ptr<orwell::game::Item> >();
std::vector< std::shared_ptr< orwell::game::Item > > orwell::game::Item::s_allItems = std::vector< std::shared_ptr< orwell::game::Item > >();

namespace orwell
{
namespace game
{

Item::Item(
		std::string const & iName,
		std::set< std::string > const & iRfids,
		boost::posix_time::milliseconds const & iTimeToCapture)
	: m_kind(Kind::RFIDS)
	, m_name(iName)
	, m_rfids(iRfids)
	, m_colour(-1)
	, m_captureState(CaptureState::PENDING)
	, m_timeToCapture(iTimeToCapture)
{
	std::string aStringRfid;
	for (std::string const & aRfid : iRfids)
	{
		aStringRfid += " " + aRfid;
	}
	ORWELL_LOG_DEBUG("new Item (" << iName << ") by Rfid(s) (" << aStringRfid.substr(1) << ")");
}

Item::Item(
		std::string const & iName,
		int32_t const iColourCode,
		boost::posix_time::milliseconds const & iTimeToCapture)
	: m_kind(Kind::COLOUR)
	, m_name(iName)
	, m_colour(iColourCode)
	, m_timeToCapture(iTimeToCapture)
{
	ORWELL_LOG_DEBUG("new Item (" << iName << ") by colour (" << iColourCode << ")");
}

Item::~Item()
{
}

void Item::InitializeStaticMaps()
{
	Item::s_itemsByRfid = std::map< std::string, std::shared_ptr< Item > >();
	Item::s_itemsByColour = std::map< int32_t, std::shared_ptr< Item > >();
}

std::string const & Item::getName() const
{
	return m_name;
}

std::set< std::string > const & Item::getRfids() const
{
	return m_rfids;
}

int32_t Item::getColour() const
{
	return m_colour;
}

std::string const & Item::getTeam() const
{
	return m_owningTeam;
}

std::shared_ptr<Item> Item::GetItemByRfid(
		std::string const & iRfid)
{
	std::shared_ptr< Item > aFound;
	std::map< std::string, std::shared_ptr< Item > >::iterator it = s_itemsByRfid.find(iRfid);
	if (it != s_itemsByRfid.end())
	{
		aFound = it->second;
	}
	else
	{
		ORWELL_LOG_ERROR("Tried to retrieve Item by RFID : " << iRfid << ". Not found");
	}
	return aFound;
}

std::shared_ptr< Item > Item::GetItemByColour(
		int32_t const iColourCode)
{
	std::shared_ptr< Item > aFound;
	std::map< std::int32_t, std::shared_ptr< Item > >::iterator it = s_itemsByColour.find(iColourCode);
	if (it != s_itemsByColour.end())
	{
		aFound = it->second;
	}
	else
	{
		ORWELL_LOG_ERROR("Tried to retrieve Item by Colour code : " << iColourCode << ". Not found");
	}
	return aFound;
}

std::shared_ptr< Item > Item::CreateItem(
		std::string const & iType,
		std::string const & iName,
		std::set< std::string > const & iRfids,
		int32_t const iColourCode,
		Ruleset const & iRuleset)
{
	if ("flag" == iType)
	{
		if (iRfids.empty() and -1 != iColourCode)
		{
			if (s_itemsByColour.find(iColourCode) != s_itemsByColour.end())
			{
				ORWELL_LOG_ERROR("Tried to add new Item by Colour code : " << iColourCode << ". Already exists.");
			}
			else
			{
				std::shared_ptr< item::Flag > aNewFlag = std::make_shared< item::Flag >(
						iName,
						iColourCode,
						iRuleset.m_timeToCapture,
						iRuleset.m_pointsOnCapture);
				s_itemsByColour[iColourCode] = aNewFlag;
				s_allItems.push_back(aNewFlag);
				return aNewFlag;
			}
		}
		else if (not iRfids.empty() and -1 == iColourCode)
		{
			std::shared_ptr< item::Flag > aNewFlag = std::make_shared< item::Flag >(
					iName,
					iRfids,
					iRuleset.m_timeToCapture,
					iRuleset.m_pointsOnCapture);
			for (std::string const & aRfid : iRfids)
			{
				if (s_itemsByRfid.find(aRfid) != s_itemsByRfid.end())
				{
					ORWELL_LOG_ERROR("Tried to add new Item by Rfid : " << aRfid << ". Already exists.");
				}
				else
				{
					ORWELL_LOG_INFO("Add flag with key '" << aRfid << "'")
					s_itemsByRfid[aRfid] = aNewFlag;
				}
			}
			s_allItems.push_back(aNewFlag);
			return aNewFlag;
		}
	}

	return nullptr;
}

std::vector< std::shared_ptr< Item > > Item::GetAllItems()
{
	return s_allItems;
}

std::string Item::toLogString() const
{
	std::stringstream aLogString;
	aLogString << *this;
	return aLogString.str();
}

void Item::startCapture(std::string const & iCapturingTeam)
{
	m_capturingTeam = iCapturingTeam;
	m_captureState = CaptureState::STARTED;
}

void Item::abortCapture()
{
	m_captureState = CaptureState::FAILED;
}

std::string const & Item::getCapturingTeam() const
{
	return m_capturingTeam;
}

CaptureState Item::getCaptureState() const
{
	return m_captureState;
}

void Item::capture(Team & ioTeam)
{
	if (ioTeam.getName() != m_owningTeam)
	{
		innerCapture(ioTeam);
		m_owningTeam = ioTeam.getName();
		m_captureState = CaptureState::SUCCEEDED;
	}
	else
	{
		// not sure that it should go back to PENDING
		m_captureState = CaptureState::PENDING;
	}
}

} // game
} // orwell

std::ostream & operator<<(
		std::ostream & ioOstream,
		orwell::game::Item const & aItem)
{
	ioOstream << "Item : " << aItem.getName();
	if (not aItem.getRfids().empty())
	{
		ioOstream << " - rfid :";
		for (std::string const & aRfid : aItem.getRfids())
		{
			ioOstream << " " << aRfid;
		}
	}
	if (aItem.getColour() >= 0)
	{
		ioOstream << " - colour : " << aItem.getColour();
	}
	return ioOstream;
}

