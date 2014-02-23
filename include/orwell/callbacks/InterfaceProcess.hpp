#pragma once

/*
 * Abstract class of which all callbacks inherit
 */

#include <memory>
#include "orwell/com/Sender.hpp"
//#include <google/protobuf/message_lite.h>
#include <log4cxx/logger.h>

namespace google
{
namespace protobuf
{
class  MessageLite;
}
}

namespace orwell {
namespace game {
class Game;
}
namespace callbacks {

class InterfaceProcess
{
	typedef std::string Key;
	typedef std::string Value;
	typedef std::pair<Key, Value> Argument;
	typedef std::vector<Argument> DictionaryOfArguments;

public:
	InterfaceProcess(std::shared_ptr< com::Sender > ioPublisher);
	InterfaceProcess(
			std::shared_ptr< com::Sender > ioPublisher,
			game::Game & ioGame);

	virtual ~InterfaceProcess();
	virtual void execute() = 0;

	void init(
			google::protobuf::MessageLite * ioMsg,
			log4cxx::LoggerPtr ioLogger,
			game::Game * ioGame = nullptr);

	void setGameContext(game::Game & ioGame);

	void insertArgument(Argument const & iArgument);

	void insertArgument(Key const & iKey, Value const & iValue);

	void removeArgument(Key const & iKey);

	Argument & accessArgument(Key const & iKey);

	Argument const & getArgument(Key const & iKey);

protected:
    std::shared_ptr< com::Sender > _publisher;

	google::protobuf::MessageLite * _msg;
	log4cxx::LoggerPtr _loggerPtr;

	game::Game * _game;

	DictionaryOfArguments _dictionary;

};

}} //namespaces

