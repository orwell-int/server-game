/*  */

#pragma once

#include <string>
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

	static std::shared_ptr<Item> GetItem(
			std::string const & iRfid,
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

};

}} //end namespace

std::ostream & operator<<(std::ostream& oOstream, const orwell::game::Item & aFlag);
