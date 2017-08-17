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

// Todo: se déconnecter du locator à la fin du progamme
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
		ClientBaseDCS();
		virtual void Init(size_t id);
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

