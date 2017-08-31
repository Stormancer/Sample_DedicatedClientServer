#pragma once

#include "Compatibility/IStormancerTask.h"
#include "Endpoint.h"
#include "ConnectionStatus.h"



namespace SampleDCS {
	class IClientDCS
	{
	public:
		static std::shared_ptr<IClientDCS> MakeClientDCS(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers);		
	
		virtual Task_ptr<void> RunClient(std::string& ticket) = 0;
		virtual Task_ptr<Endpoint> TravelToMap(std::string mapID) = 0;
		virtual void Tick() = 0;
		virtual int GetConnectionStatus() = 0;
		virtual void SetConnectionStatus(int status) = 0;
		virtual void SetNextMap(std::string mapId) = 0;
		virtual std::string GetNextMap() = 0;

		//Event
		//virtual void OnConnectionStatusChange(const std::function<void(int)>& connectionStatusChange);
		std::function<void(int)> OnConnectionStatusChange;
	
	};
}