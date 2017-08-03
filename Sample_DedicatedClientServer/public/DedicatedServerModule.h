#pragma once
#include "Compatibility/IStormancerTask.h"

namespace Stormancer
{
	class P2PTunnel;
}

struct Endpoint
{
	std::string host;
	unsigned short port;
};
class DedicatedServerModule
{
public:
	DedicatedServerModule(size_t id, const std::string& endPoint,std::string accountID, std::string applicationName, int maxPeers);
	Task_ptr<Endpoint> startClient(std::string ticket);

	Task_ptr<void> startServer(std::string connectionToken, std::function<void(Endpoint)> startServerCallback, std::function<void()> stopServerCallback);

	void Tick();
private:
	size_t _id;
	std::shared_ptr<Stormancer::P2PTunnel> _p2pTunnel;
};