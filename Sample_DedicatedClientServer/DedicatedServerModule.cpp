#include "stdafx.h"
#include "public\DedicatedServerModule.h"
#include "online\GameSessionPluginP2P.h"
#include "Authentication\AuthenticationPlugin.h"
#include "Compatibility\pplxtasks_noexcept.h"
#include "IClientFactory.h"

pplx::task<std::shared_ptr<Stormancer::P2PTunnel>> runClient(std::shared_ptr<Stormancer::Client> client, std::string ticket);
pplx::task<void> runServer(
	std::shared_ptr<Stormancer::Client> client,
	std::string connectionToken,
	std::function<void(std::shared_ptr<Stormancer::P2PTunnel>)>  onStartServerReceived,
	std::function<void()> onStopServerReceived);



DedicatedServerModule::DedicatedServerModule(size_t id,const std::string& endPoint) :
	_id(id)
{
	Stormancer::IClientFactory::SetConfig(id, [endPoint]() {
		auto logger = std::make_shared<Stormancer::ConsoleLogger>();
		auto config = Stormancer::Configuration::create(endPoint, "samples-jm", "sample-dedicated");
		config->actionDispatcher = std::make_shared<Stormancer::MainThreadActionDispatcher>();
		config->logger = logger;
		//Adds the auth plugin to the client. It enable the AuthenticationService to easily interact with the server authentication plugin.
		config->addPlugin(new Stormancer::AuthenticationPlugin());
		config->addPlugin(new Stormancer::GameSessionPluginP2P());
		return config;
	});
}

Task_ptr<Endpoint> DedicatedServerModule::startClient(std::string ticket)
{

	auto client = Stormancer::IClientFactory::GetClient(_id);
	auto logger = client->dependencyResolver()->resolve<Stormancer::ILogger>();
	auto dispatcher = client->dependencyResolver()->resolve<Stormancer::IActionDispatcher>();
	return Stormancer::Task<Endpoint>::create(
		runClient(client, ticket).then([](std::shared_ptr<Stormancer::P2PTunnel> t) { return Endpoint{ t->ip,t->port }; }),
		logger,
		dispatcher);
}

Task_ptr<void> DedicatedServerModule::startServer(std::string connectionToken, std::function<void(Endpoint)> startServerCallback, std::function<void()> stopServerCallback)
{
	auto client = Stormancer::IClientFactory::GetClient(_id);
	auto logger = client->dependencyResolver()->resolve<Stormancer::ILogger>();
	auto dispatcher = client->dependencyResolver()->resolve<Stormancer::IActionDispatcher>();
	return Stormancer::Task<void>::create(
		runServer(client, connectionToken, [startServerCallback](std::shared_ptr<Stormancer::P2PTunnel> tunnel) {
		Endpoint endpoint;
		endpoint.host = tunnel->ip;
		endpoint.port = tunnel->port;
		startServerCallback(endpoint);
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
