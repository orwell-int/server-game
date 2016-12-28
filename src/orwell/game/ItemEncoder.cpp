#include "orwell/game/ItemEncoder.hpp"

#include "orwell/game/Item.hpp"
#include "server-game.pb.h"

namespace orwell
{
namespace game
{

ItemEncoder::ItemEncoder()
{
}

ItemEncoder::~ItemEncoder()
{
}

void ItemEncoder::encode(orwell::messages::Item & ioItem) const
{
	ioItem.set_name(getItem().getName());
	if (not getItem().getTeam().empty())
	{
		ioItem.set_owner(getItem().getTeam());
	}
	switch (getItem().getCaptureState())
	{
		case CaptureState::STARTED:
		{
			ioItem.set_capture_status(orwell::messages::STARTED);
			ioItem.set_capturer(getItem().getCapturingTeam());
			break;
		}
		case CaptureState::FAILED:
		{
			ioItem.set_capture_status(orwell::messages::FAILED);
			break;
		}
		case CaptureState::SUCCEEDED:
		{
			ioItem.set_capture_status(orwell::messages::SUCCEEDED);
			break;
		}
		case CaptureState::PENDING:
		{
			// just do not include it
			break;
		}
	}
}

} // namespace game
} // namespace orwell
