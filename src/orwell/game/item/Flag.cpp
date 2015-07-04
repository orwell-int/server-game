#include "orwell/game/item/Flag.hpp"

#include <ostream>

#include "orwell/game/Team.hpp"

namespace orwell
{
namespace game
{
namespace item
{

Flag::Flag(
		std::string const & iName,
		std::set< std::string > const & iRfids,
		boost::posix_time::milliseconds const & iTimeToCapture,
		uint32_t const iPointsOnCapture)
	: Item(iName, iRfids, iTimeToCapture)
	, m_pointsOnCapture(iPointsOnCapture)
{
}

Flag::Flag(
		std::string const & iName,
		int32_t const iColourCode,
		boost::posix_time::milliseconds const & iTimeToCapture,
		uint32_t const iPointsOnCapture)
	: Item(iName, iColourCode, iTimeToCapture)
	, m_pointsOnCapture(iPointsOnCapture)
{
}

Flag::~Flag()
{
}

void Flag::innerCapture(Team & ioTeam)
{
	ioTeam.increaseScore(m_pointsOnCapture);
}

std::ostream& operator<<(std::ostream& oOstream, const Flag & aFlag)
{
	return oOstream << " this is a flag";
}

} // item
} // game
} // orwell

