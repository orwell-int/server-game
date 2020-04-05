#pragma once

#include <memory>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "orwell/IServer.hpp"

#include "orwell/callbacks/ProcessDecider.hpp"

#include "orwell/game/Game.hpp"
#include "orwell/com/Channel.hpp"

#include <zmq.hpp>

namespace orwell
{

namespace com
{
class Receiver;
class Sender;
}

namespace proxy
{
class IAgentProxy;
}

class Server : public IServer
{
public:
	/// \param iTicDuration
	///  Time in milliseconds between to GameState.
	///
	/// \param iGameDuration
	///  Duration of the game in seconds.
	Server(
			support::ISystemProxy const & iSystemProxy,
			orwell::proxy::IAgentProxy & ioAgentProxy,
			game::Ruleset const & iRuleset,
			std::string const & iAgentUrl =  "tcp://*:9003",
			std::string const & iPullUrl = "tcp://*:9000",
			std::string const & iPublishUrl = "tcp://*:9001",
			std::string const & iReplyUrl = "tcp://*:9002",
			long const iTicDuration = 500,
			uint32_t const iGameDuration = 300);

	~Server();

	/// processMessageIfAvailable
	bool processMessageIfAvailable() override;
	///// run the broadcast receiver
	//void runBroadcastReceiver();
	/// Wait for 1 message and process it. Execute timed operations if needed.
	void loopUntilOneMessageIsProcessed() override;
	/// Loop eternaly to process all incoming messages.
	void loop() override;
	/// Correctly stop the server
	void stop() override;

	orwell::game::Game & accessContext() override;

	void feedAgentProxy(bool const iBlocking) override;

private:
	bool receive_and_process(
			orwell::com::Channel const iChannel,
			orwell::com::RawMessage & ioMessage);

	zmq::context_t m_zmqContext;
	orwell::proxy::IAgentProxy & m_agentProxy;
	std::shared_ptr< com::Socket > m_agentSocket;
	std::shared_ptr< com::Receiver > m_puller;
	std::shared_ptr< com::Sender > m_publisher;
	std::shared_ptr< com::Socket > m_replier;
	orwell::game::Game m_game;
	orwell::callbacks::ProcessDecider m_decider;

	std::map< std::string, std::shared_ptr< com::Socket > > m_serverCommandSockets;

	boost::posix_time::time_duration const m_ticDuration;
	boost::posix_time::ptime m_previousTic;

	bool m_mainLoopRunning;
	bool m_forcedStop;
	std::vector< orwell::com::Channel > m_channels;
};

}
