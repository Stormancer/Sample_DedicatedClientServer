#pragma once
#include "Compatibility/IStormancerTask.h"

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

enum class ConnectionStatus
{
	Authenticating,
	FindLocator,
	ConnectingLocator,
	FindingGameSession,
	ConnectingToGameSession,
	WaitingServer,
	OpenningConnection,
	Connecting,
};

class DedicatedServerModule
{
public:
	DedicatedServerModule(size_t id, const std::string& endPoint,std::string accountID, std::string applicationName, int maxPeers);
	
	Task_ptr<Endpoint> startClient(std::string ticket);
	Task_ptr<void> startServer(std::string connectionToken, std::function<void(Endpoint)> startServerCallback, std::function<void()> stopServerCallback);	

	ConnectionStatus clientStatus;

	std::function<void(std::string)> updateLog;
	std::function<void(int)> updateConnectionStatus;

	void Tick();
private:
	size_t _id;
	std::shared_ptr<Stormancer::P2PTunnel> _p2pTunnel;
};