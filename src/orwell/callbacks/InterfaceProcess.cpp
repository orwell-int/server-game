#include "orwell/callbacks/InterfaceProcess.hpp"
#include <stdexcept>
#include "orwell/game/Game.hpp"

namespace orwell {
namespace callbacks {

InterfaceProcess::InterfaceProcess()
{
    
}

InterfaceProcess::InterfaceProcess(game::Game & ioCtx)
{
    _ctx = &ioCtx;
}
    
InterfaceProcess::~InterfaceProcess ()
{

}
    
void InterfaceProcess::setGameContext(game::Game &ioCtx)
{
    _ctx = &ioCtx;
}
    
void InterfaceProcess::init(google::protobuf::MessageLite *ioMsg, log4cxx::LoggerPtr ioLogger, game::Game * ioCtx)
{
    _msg = ioMsg;
    _loggerPtr = ioLogger;
    
    if (ioCtx /*!= nullptr*/)
    {
        _ctx = ioCtx;
    }
}
    
void InterfaceProcess::insertArgument(const Argument &iArgument)
{
    _dictionary.push_back(iArgument);
}
    
void InterfaceProcess::insertArgument(const Key &iKey, const Value &iValue)
{
    Argument anArgument(iKey, iValue);
    insertArgument(anArgument);
}
    
void InterfaceProcess::removeArgument(const Key &iKey)
{
    bool found(false);
    auto anIterator = _dictionary.begin();
    
    for (Argument const & anArgument: _dictionary)
    {
        if (anArgument.first == iKey)
        {
            found = true;
            break;
        }
        ++anIterator;
    }
    
    if (found)
    {
        _dictionary.erase(anIterator);
    }
}
    
InterfaceProcess::Argument const & InterfaceProcess::getArgument(const Key &iKey)
{
    for (Argument const & anArgument: _dictionary)
    {
        if (anArgument.first == iKey)
            return anArgument;
    }
    
    throw 1;
}
    
InterfaceProcess::Argument & InterfaceProcess::accessArgument(const Key &iKey)
{
    for (Argument & anArgument: _dictionary)
    {
        if (anArgument.first == iKey)
            return anArgument;
    }
    
    throw 1;
}

}} //namespaces
