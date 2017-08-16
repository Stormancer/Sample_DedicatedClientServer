#include "stdafx.h"
#include "public\DedicatedServerModule.h"
#include "online\GameSessionPluginP2P.h"
#include "Authentication\AuthenticationPlugin.h"
#include "Compatibility\pplxtasks_noexcept.h"
#include "IClientFactory.h"
#include "Windows.h"

pplx::task<void> runServer(
	std::shared_ptr<Stormancer::Client> client,
	std::string connectionToken,
	std::function<void(std::shared_ptr<Stormancer::P2PTunnel>)>  onStartServerReceived,
	std::function<void()> onStopServerReceived);

DedicatedServerModule::DedicatedServerModule(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers):
	_id(id)
{
	
}

Task_ptr<Endpoint> DedicatedServerModule::startClient(std::string ticket, std::string targetMap)
{


}

Task_ptr<void> DedicatedServerModule::startServer(std::string connectionToken, std::function<void(Endpoint)> startServerCallback, std::function<void()> stopServerCallback)
{
	auto client = Stormancer::IClientFactory::GetClient(_id);
	auto logger = client->dependencyResolver()->resolve<Stormancer::ILogger>();
	auto dispatcher = client->dependencyResolver()->resolve<Stormancer::IActionDispatcher>();
	return Stormancer::Task<void>::create(
		runServer(client, connectionToken, [startServerCallback, this](std::shared_ptr<Stormancer::P2PTunnel> tunnel) {
		Endpoint endpoint;
		endpoint.host = tunnel->ip;
		endpoint.port = tunnel->port;
		startServerCallback(endpoint);
		_p2pTunnel = tunnel;
	}, stopServerCallback),
		logger,
		dispatcher);
}

void DedicatedServerModule::Tick()
{
	auto client = Stormancer::IClientFactory::GetClient(_id);
	auto d = client->dependencyResolver()->resolve<Stormancer::IActionDispatcher>();
	auto dispatcher = std::dynamic_pointer_cast<Stormancer::MainThreadActionDispatcher>(d);
	dispatcher->update((std::chrono::milliseconds)10);
}
