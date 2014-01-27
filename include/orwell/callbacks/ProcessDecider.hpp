#pragma once

#include <RawMessage.hpp>

namespace orwell {
namespace game {
	class Game;
}
namespace callbacks {

class ProcessDecider
{
	public:
		static void Process( com::RawMessage const & iMessage,
                             game::Game & ioCtx);

	private:
        ProcessDecider();
};

}} //end namespace

