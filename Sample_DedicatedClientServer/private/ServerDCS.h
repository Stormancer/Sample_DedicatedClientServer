#pragma once

#include "Compatibility/IStormancerTask.h"
#include "IServerDCS.h"

namespace SampleDCS {
	class ServerDCS : public IServerDCS
	{
	public:
		ServerDCS(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers);
		
		Task_ptr<void> RunServer(std::string connectionToken, std::function<void(Endpoint)> onStartServerReceived, std::function<void()> onStopServerReceived) override;

		Task_ptr<void> UpdateShutdownMode(UpdateShutdownModeParameter param) override;

		void Tick() override;

		size_t _OnlineClientID;
	private:
		pplx::task<void> _RunServer(std::string connectionToken, std::function<void(Endpoint)> onStartServerReceived, std::function<void()> onStopServerReceived);
		std::string _mapToken;
		std::shared_ptr<Stormancer::Client> _stormancerClient;
		std::shared_ptr<Stormancer::IActionDispatcher> _actionDispatcher;
		std::shared_ptr<Stormancer::ILogger> _logger;
		std::shared_ptr<Stormancer::P2PTunnel> _p2pTunnel;
		std::shared_ptr<Stormancer::Scene> _currentScene;
	};
}