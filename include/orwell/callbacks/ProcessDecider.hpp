#pragma once

#include <RawMessage.hpp>
#include "Sender.hpp"

namespace orwell {
namespace game {
	class Game;
}
namespace callbacks {

class ProcessDecider
{
	public:
		static void Process( com::RawMessage const & iMessage,
                             game::Game & ioGame,
                             std::shared_ptr< com::Sender > ioPublisher);

	private:
        ProcessDecider();
};

}} //end namespace

