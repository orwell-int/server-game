#include "orwell/game/Item.hpp"

#include "orwell/game/item/Flag.hpp"
#include <sstream>

namespace orwell {
namespace game
{

Item::Item(
		std::string const & iName,
		std::string const & iRfid) :
		m_name(iName),
		m_rfid(iRfid)
{
}

Item::Item(
		std::string const & iName,
		int32_t const iColorCode):
		m_name(iName),
		m_color(iColorCode)
{
}

Item::~Item()
{}

std::string Item::getName() const
{
	return m_name;
}

std::string Item::getRfid() const
{
	return m_rfid;
}

int32_t Item::getColor() const
{
	return m_color;
}

std::shared_ptr<Item> Item::GetItem(
			std::string const & iRfid,
			int32_t const iColorCode)
{
	std::shared_ptr<Item> p1 (nullptr);
	return p1;
}

std::shared_ptr<Item> Item::CreateItem(
		std::string const & iType,
		std::string const & iName,
		std::string const & iRfid,
		int32_t const iColorCode)
{
	if (iType == "flag")
	{
		if (iRfid.empty() and iColorCode != -1)
		{
			return std::make_shared<item::Flag> (iName, iColorCode);
		}
		else if (not iRfid.empty() and iColorCode == -1)
		{
			return std::make_shared<item::Flag> (iName, iRfid);
		}
	}

	return nullptr;
}

std::string Item::toLogString() const
{
	std::stringstream aLogString;
	aLogString << *this;
	return aLogString.str();
}

}} //namespaces

std::ostream & operator<<(std::ostream& oOstream, const orwell::game::Item & aItem)
{
	oOstream << "Item : " << aItem.getName();
	if (not aItem.getRfid().empty())
	{
		oOstream << " - rfid : " << aItem.getRfid();
	}
	if (aItem.getColor() >= 0)
	{
		oOstream << " - color : " << aItem.getColor();
	}
	return oOstream ;
}



