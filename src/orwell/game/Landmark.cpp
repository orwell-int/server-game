#include "orwell/game/Landmark.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include <boost/property_tree/ptree.hpp>

#include <sstream>

namespace orwell
{
namespace game
{

Landmark::Landmark(
		Coordinates const & iPosition,
		RGBColour const & iColour)
	: m_position(iPosition)
	, m_colour(iColour)
{
}

Landmark::~Landmark()
{
}

Coordinates const & Landmark::getPosition() const
{
	return m_position;
}

RGBColour const & Landmark::getColour() const
{
	return m_colour;
}

bool Landmark::operator==(orwell::game::Landmark const & iOther) const
{
	return ((getPosition() == iOther.getPosition())
		and (getColour() == iOther.getColour()));
}

Landmark Landmark::ParseConfig(
		std::string const & iMapLimitName,
		boost::property_tree::ptree const & iPtree)
{
	return Landmark(
			Coordinates::ParseConfig(iMapLimitName, iPtree), 
			RGBColour::ParseConfig(iMapLimitName, iPtree));
}

} // game
} // orwell

std::ostream & operator<<(
		std::ostream & ioOstream,
		orwell::game::Landmark const & iLandmark)
{
	ioOstream << "Landmark : " << iLandmark.getPosition();
	ioOstream << " ; " << iLandmark.getColour();
	return ioOstream;
}
