#include "orwell/game/Coordinates.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include <boost/property_tree/ptree.hpp>
#include <sstream>

namespace orwell
{
namespace game
{

Coordinates::Coordinates(
		uint64_t const iX,
		uint64_t const iY)
	: m_x(iX)
	, m_y(iY)
{
}

Coordinates::~Coordinates()
{
}

uint64_t Coordinates::getX() const
{
	return m_x;
}

uint64_t Coordinates::getY() const
{
	return m_y;
}

Coordinates Coordinates::ParseConfig(
		std::string const & iCoordinatesName,
		boost::property_tree::ptree const & iPtree)
{
	uint64_t aX = iPtree.get< uint64_t >(iCoordinatesName + ".x");
	uint64_t aY = iPtree.get< uint64_t >(iCoordinatesName + ".y");
	return Coordinates(aX, aY);
}
} // game
} // orwell

std::ostream & operator<<(
		std::ostream & ioOstream,
		orwell::game::Coordinates const & aCoordinates)
{
	ioOstream << "Coordinates : " << aCoordinates.getX() << " ; " << aCoordinates.getY();
	return ioOstream;
}

bool operator==(
		orwell::game::Coordinates const & iLeft,
		orwell::game::Coordinates const & iRight)
{
	return ((iLeft.getX() == iRight.getX())
		and (iLeft.getY() == iRight.getY()));
}
