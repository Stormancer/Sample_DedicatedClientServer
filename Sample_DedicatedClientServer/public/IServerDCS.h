#pragma once

#include "Endpoint.h"
#include "Compatibility/IStormancerTask.h"

namespace SampleDCS {
	class IServerDCS 
	{
	public:
		static std::shared_ptr<IServerDCS> MakeServerDCS(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers);

		virtual Task_ptr<void> RunServer(std::string connectionToken, std::function<void(Endpoint)> onStartServerReceived, std::function<void()> onStopServerReceived) = 0;

		virtual void Tick() {}
	};
}