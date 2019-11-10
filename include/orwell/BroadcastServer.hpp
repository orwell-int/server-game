#pragma once

#include <string>

namespace orwell
{

class BroadcastServer
{
public:
	BroadcastServer(
			uint16_t const iBroadcastPort,
			std::string const & iPullerUrl,
			std::string const & iPublisherUrl,
			std::string const & iReplierUrl,
			std::string const & iAgentUrl);

	~BroadcastServer();

	/// run the broadcast receiver
	void runBroadcastReceiver();
	/// Correctly stop the server
	void stop();

private:
	bool _mainLoopRunning;
	bool _forcedStop;
	uint16_t const m_broadcastPort;
	std::string const _pullerUrl;
	std::string const _publisherUrl;
	std::string const _replierUrl;
	std::string const _agentUrl;
};

}

