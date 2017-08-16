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

/**
Struc containging the endpoint information
*/
struct Endpoint
{
	std::string host;
	unsigned short port;
};

class ClientBase
{
public:
	ClientBase();
	std::function<void(int)> OnConnectionStatusChange;

protected:
	std::shared_ptr<Stormancer::P2PTunnel> _p2pTunnel;
};

