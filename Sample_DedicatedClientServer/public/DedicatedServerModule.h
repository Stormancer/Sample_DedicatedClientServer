#pragma once
#include "Compatibility/IStormancerTask.h"
#include "OnlineClient.h"

namespace Stormancer
{
	class P2PTunnel;
}



class DedicatedServerModule
{
public:
	DedicatedServerModule(size_t id, const std::string& endPoint,std::string accountID, std::string applicationName, int maxPeers);
	
	Task_ptr<Endpoint> startClient(std::string ticket, std::string targetMap);
	Task_ptr<void> startServer(std::string connectionToken, std::function<void(Endpoint)> startServerCallback, std::function<void()> stopServerCallback);	

	ConnectionStatus clientStatus;

	/*std::function<void(std::string)> updateLog;

	std::function<void(ConnectionStatus)> clientCallback;*/
	std::shared_ptr<OnlineClient> _onlineClient;

	void Tick();
private:
	size_t _id;

};