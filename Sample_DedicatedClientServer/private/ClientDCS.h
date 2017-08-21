#pragma once

#include "stormancer.h"
#include "Endpoint.h"
#include "Compatibility/IStormancerTask.h"
#include "IClientDCS.h"

namespace Stormancer
{
	class P2PTunnel;
}

namespace SampleDCS {
	//Forward declaration

	//My class
	class ClientDCS : public IClientDCS
	{	
	public:
		ClientDCS(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers);
		
		// IClientImplementation
		Task_ptr<void> RunClient(std::string& ticket) override;
		Task_ptr<Endpoint> TravelToMap(std::string mapID) override;
		void Tick() override;

	private:
		
		void Init(size_t id);
		pplx::task<void> _RunClient(std::string& ticket);
		pplx::task<Endpoint> _TravelToMap(std::string mapID);
		pplx::task<Endpoint> _ConnectToMap(std::string mapID);
		pplx::task<void> _DisconnectFromMap(std::string mapID);

		size_t _OnlineClientID;
		std::shared_ptr<Stormancer::IActionDispatcher> _actionDispatcher;
		std::shared_ptr<Stormancer::ILogger> _logger;
		std::shared_ptr<Stormancer::P2PTunnel> _p2pTunnel;

		Stormancer::Scene_ptr currentMap;

		ConnectionStatus _clientStatus;
		std::shared_ptr<Stormancer::Client> _stormancerClient;
		//std::vector<std::function<void(int)>> OnConnectionStatusChangeDelegate;
	};
}



