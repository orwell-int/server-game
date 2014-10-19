#include "orwell/game/item/Flag.hpp"

#include <ostream>

namespace orwell {
namespace game {
namespace item
{

Flag::Flag(std::string const & iName,
		std::string const & iRfid) :
		Item(iName, iRfid)
{
}

Flag::Flag(std::string const & iName,
		int32_t const iColorCode) :
		Item(iName, iColorCode)
{
}

Flag::~Flag()
{}

std::ostream& operator<<(std::ostream& oOstream, const Flag & aFlag)
{
	return oOstream << " this is a flag";
}



}}} //namespaces



