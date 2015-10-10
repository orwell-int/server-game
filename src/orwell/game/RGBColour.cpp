#include "orwell/game/RGBColour.hpp"

#include <boost/property_tree/ptree.hpp>

namespace orwell
{
namespace game
{

RGBColour::RGBColour(
		uint8_t const iR,
		uint8_t const iG,
		uint8_t const iB)
	: m_r(iR)
	, m_g(iG)
	, m_b(iB)
{
}

uint8_t RGBColour::getRed() const
{
	return m_r;
}

uint8_t RGBColour::getGreen() const
{
	return m_g;
}

uint8_t RGBColour::getBlue() const
{
	return m_b;
}

bool RGBColour::operator==(RGBColour const & iOther) const
{
	return ((m_r == iOther.m_r) && (m_g == iOther.m_g) && (m_b == iOther.m_b));

}

RGBColour RGBColour::ParseConfig(
		std::string const & iMapLimitName,
		boost::property_tree::ptree const & iPtree)
{
	uint8_t const aR = iPtree.get< uint8_t >(iMapLimitName + ".r");
	uint8_t const aG = iPtree.get< uint8_t >(iMapLimitName + ".g");
	uint8_t const aB = iPtree.get< uint8_t >(iMapLimitName + ".b");
	return RGBColour(aR, aG, aB);
}

}
}

std::ostream & operator<<(
		std::ostream & ioOstream,
		orwell::game::RGBColour const & iColour)
{
	ioOstream << "Colour : { " <<
		"R = " << iColour.getRed() << " ; " <<
		"G = " << iColour.getGreen() << " ; " <<
		"B = " << iColour.getBlue() << " }";
	return ioOstream;
}
