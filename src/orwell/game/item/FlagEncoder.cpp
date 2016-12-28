#include "orwell/game/item/FlagEncoder.hpp"

#include "server-game.pb.h"

#include "orwell/game/item/Flag.hpp"

namespace orwell
{
namespace game
{
namespace item
{

FlagEncoder::FlagEncoder(Flag const & iFlag)
	: m_flag(iFlag)
{
}

FlagEncoder::~FlagEncoder()
{
}

void FlagEncoder::encode(orwell::messages::Item & ioItem) const
{
	ItemEncoder::encode(ioItem);
	ioItem.set_type(::orwell::messages::FLAG);
}

Item const & FlagEncoder::getItem() const
{
	return m_flag;
}

} // namespace item
} // namespace game
} // namespace orwell
