#pragma once

namespace orwell
{
namespace messages
{
class Item;
} // namespace messages
namespace game
{
class Item;

class ItemEncoder
{
protected:
	ItemEncoder();

	virtual Item const & getItem() const = 0;

public:
	virtual ~ItemEncoder();

	virtual void encode(orwell::messages::Item & ioItem) const;
};

} // namespace game
} // namespace orwell
