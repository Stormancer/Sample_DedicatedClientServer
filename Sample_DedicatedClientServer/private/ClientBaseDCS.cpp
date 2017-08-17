#include "stdafx.h"
#include "Authentication\AuthenticationPlugin.h"
#include "IClientFactory.h"
#include "Compatibility\pplxtasks_noexcept.h"
#include "ClientBaseDCS.h"

namespace SampleDCS
{
	ClientBaseDCS::ClientBaseDCS(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers)
	{
		SetConfig(id, endPoint, accountID, applicationName, maxPeers);

		_client = Stormancer::IClientFactory::GetClient(id);
		_actionDispatcher = _client->dependencyResolver()->resolve<Stormancer::IActionDispatcher>();
		_logger = _client->dependencyResolver()->resolve<Stormancer::ILogger>();
	}

	void ClientBaseDCS::Tick()
	{
		auto dispatcher = std::dynamic_pointer_cast<Stormancer::MainThreadActionDispatcher>(_actionDispatcher);
		dispatcher->update((std::chrono::milliseconds)10);
	}

	void ClientBaseDCS::SetConfig(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers) 
	{
		_logger->log(Stormancer::LogLevel::Error, "Error", "SetConfig is not implemented");
	}
}
