#include "stdafx.h"
#include "Authentication\AuthenticationPlugin.h"
#include "IClientFactory.h"
#include "Compatibility\pplxtasks_noexcept.h"
#include "ClientBaseDCS.h"

namespace SampleDCS
{
	ClientBaseDCS::ClientBaseDCS()
	{
	}

	void ClientBaseDCS::Tick()
	{
		auto dispatcher = std::dynamic_pointer_cast<Stormancer::MainThreadActionDispatcher>(_actionDispatcher);
		dispatcher->update((std::chrono::milliseconds)10);
	}

	void ClientBaseDCS::Init(size_t id)
	{
		_client = Stormancer::IClientFactory::GetClient(id);
		_actionDispatcher = _client->dependencyResolver()->resolve<Stormancer::IActionDispatcher>();
		_logger = _client->dependencyResolver()->resolve<Stormancer::ILogger>();
	}


}
