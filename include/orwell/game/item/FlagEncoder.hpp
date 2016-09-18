#pragma once

#include "orwell/game/ItemEncoder.hpp"

namespace orwell
{
namespace game
{
namespace item
{
class Flag;

class FlagEncoder : public ItemEncoder
{
public:
	FlagEncoder(Flag const & iFlag);

	~FlagEncoder();

	void encode(orwell::messages::Item & ioItem) const override;

protected:
	Item const & getItem() const override;

private:
	Flag const & m_flag;
};

} // namespace item
} // namespace game
} // namespace orwell
