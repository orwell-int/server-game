#pragma once

#include <RawMessage.hpp>
#include <InterfaceProcess.hpp>
#include <map>

namespace orwell {
namespace game {
	class Game;
}
namespace callbacks {

class ProcessDecider
{
    typedef std::pair<std::string, std::unique_ptr<InterfaceProcess>> Couple;

	public:
        ProcessDecider();
        ~ProcessDecider();
		static void Process( com::RawMessage const & iMessage,
                             game::Game & ioCtx);
        void process( com::RawMessage const & iMessage,
                             game::Game & ioCtx);

	private:
        std::map<std::string, std::unique_ptr<InterfaceProcess>> _map;
};

}} //end namespace

