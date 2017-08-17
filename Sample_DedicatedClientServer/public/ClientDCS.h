#pragma once

#include "Authentication/AuthenticationService.h"
#include "Compatibility/IStormancerTask.h"
#include "ClientBaseDCS.h"

namespace SampleDCS {
	class ClientDCS : public ClientBaseDCS
	{
	public:
		ClientDCS(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers);		

		pplx::task<void> RunClient(std::string& ticket);
		Task_ptr<Endpoint> ConnectToMap(std::string mapId);
		pplx::task<void> DisconnectFromMap();

		size_t _OnlineClientID;

	private:
		std::string _mapToken;
	};

}