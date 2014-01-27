#pragma once

/*
 * Abstract class of which all callbacks inherit
 */

namespace orwell {
namespace game {
	class Game;
}
namespace callbacks {

class InterfaceProcess
{

public:
    InterfaceProcess(game::Game & ioCtx);
    virtual ~InterfaceProcess ();

    virtual void execute() = 0;


protected:
    game::Game & _ctx;

};

}} //namespaces
