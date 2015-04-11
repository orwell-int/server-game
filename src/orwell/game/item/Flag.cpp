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
		std::string const & iRfid,
		boost::posix_time::milliseconds const & iTimeToCapture,
		uint32_t const iPointsOnCapture)
	: Item(iName, iRfid, iTimeToCapture)
	, m_pointsOnCapture(iPointsOnCapture)
{
}

Flag::Flag(
		std::string const & iName,
		int32_t const iColorCode,
		boost::posix_time::milliseconds const & iTimeToCapture,
		uint32_t const iPointsOnCapture)
	: Item(iName, iColorCode, iTimeToCapture)
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

