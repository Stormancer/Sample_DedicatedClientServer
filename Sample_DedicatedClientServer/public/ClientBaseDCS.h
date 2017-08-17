#pragma once

enum class ConnectionStatus
{
	Authenticating,
	FindLocator,
	ConnectingLocator,
	FindingGameSession,
	ConnectingToGameSession,
	WaitingServer,
	OpeningConnection,
	Connecting,
};

namespace Stormancer
{
	class P2PTunnel;
}

namespace SampleDCS {
	/**
	Struc containging the endpoint information
	*/
	struct Endpoint
	{
		std::string host;
		unsigned short port;
	};

	class ClientBaseDCS
	{
	public:
		ClientBaseDCS(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers);
		virtual void SetConfig(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers);
		std::function<void(int)> OnConnectionStatusChange;
		void Tick();
	protected:
		std::shared_ptr<Stormancer::Client> _client;
		std::shared_ptr<Stormancer::P2PTunnel> _p2pTunnel;
		std::shared_ptr<Stormancer::IActionDispatcher> _actionDispatcher;
		std::shared_ptr<Stormancer::ILogger> _logger;

		ConnectionStatus clientStatus;
	};
}

