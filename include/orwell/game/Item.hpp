/*  */

#pragma once

#include <string>
#include <map>
#include <memory>
#include <ostream>

namespace orwell {
namespace game
{

class Item
{
protected:
	Item(std::string const & iName,
			std::string const & iRfid);
	Item(std::string const & iName,
			int32_t const iColorCode);

	virtual ~Item();

public:
	std::string getName() const;
	std::string getRfid() const;
	int32_t getColor() const;

	static void InitializeStaticMaps();

	static std::shared_ptr<Item> GetItemByRfid(
			std::string const & iRfid);

	static std::shared_ptr<Item> GetItemByColor(
			int32_t const iColorCode);

	static std::shared_ptr<Item> CreateItem(
			std::string const & iType,
			std::string const & iName,
			std::string const & iRfid,
			int32_t const iColorCode);

	virtual std::string toLogString() const;

private:
	std::string m_name;
	std::string m_rfid;
	int32_t m_color;

	static std::map<std::string, std::shared_ptr<Item> > s_itemsByRfid;
	static std::map<int32_t, std::shared_ptr<Item> > s_itemsByColor;

};

}} //end namespace

std::ostream & operator<<(std::ostream& oOstream, const orwell::game::Item & aFlag);
