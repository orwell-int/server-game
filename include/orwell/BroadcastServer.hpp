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
	bool m_mainLoopRunning;
	bool m_forcedStop;
	uint16_t const m_broadcastPort;
	std::string const m_pullerUrl;
	std::string const m_publisherUrl;
	std::string const m_replierUrl;
	std::string const m_agentUrl;
};

}

