#pragma once

#include "Authentication/AuthenticationService.h"
#include "Compatibility/IStormancerTask.h"
#include "../online/GameSessionServiceP2P.h"
#include "ClientBaseDCS.h"

namespace SampleDCS {
	class ServerDCS : public ClientBaseDCS
	{
	public:
		ServerDCS(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers);
		
		pplx::task<void> RunServer(std::string connectionToken, std::function<void(Endpoint)> onStartServerReceived, std::function<void()> onStopServerReceived);

		size_t _OnlineClientID;
	private:
		std::string _mapToken;
	};
}