#pragma once

/*
 * Abstract class of which all callbacks inherit
 */

#include <memory>
#include "Sender.hpp"

namespace orwell {
namespace game {
	class Game;
}
namespace callbacks {

class InterfaceProcess
{

public:
    InterfaceProcess(game::Game & ioCtx, std::shared_ptr< com::Sender > ioPublisher);
    virtual ~InterfaceProcess ();

    virtual void execute() = 0;


protected:
    game::Game & _game;
    std::shared_ptr< com::Sender > _publisher;

};

}} //namespaces
