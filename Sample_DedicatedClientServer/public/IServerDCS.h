#pragma once

#include "Endpoint.h"
#include "msgpack_define_DCS.h"
#include "Compatibility/IStormancerTask.h"

namespace SampleDCS {

	enum class ShutdownMode:int
	{
		NoPlayerLeft,
		SceneShutdown
	};

	class UpdateShutdownModeParameter
	{
	public :
		ShutdownMode mode;
		int keepServerAliveFor;

		MSGPACK_DEFINE(mode, keepServerAliveFor)
	};


	class IServerDCS 
	{
	public:
		static std::shared_ptr<IServerDCS> MakeServerDCS(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers);

		virtual Task_ptr<void> RunServer(std::string connectionToken, std::function<void(Endpoint)> onStartServerReceived, std::function<void()> onStopServerReceived) = 0;

		virtual Task_ptr<void> UpdateShutdownMode(UpdateShutdownModeParameter updateParam) = 0;

		virtual void Tick() {}
	};
}

MSGPACK_ADD_ENUM(SampleDCS::ShutdownMode);