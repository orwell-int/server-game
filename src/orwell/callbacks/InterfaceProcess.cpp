#include <InterfaceProcess.hpp>

#include <Game.hpp>

namespace orwell {
namespace callbacks {

InterfaceProcess::InterfaceProcess(game::Game & ioCtx, std::shared_ptr< com::Sender > ioPublisher) :
		_game(ioCtx),
		_publisher( ioPublisher )
{
}

InterfaceProcess::~InterfaceProcess ()
{

}

}} //namespaces
