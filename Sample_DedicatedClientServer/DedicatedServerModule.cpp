#include "stdafx.h"
#include "public\DedicatedServerModule.h"
#include "online\GameSessionPluginP2P.h"
#include "Authentication\AuthenticationPlugin.h"
#include "Compatibility\pplxtasks_noexcept.h"
#include "IClientFactory.h"

#include "Windows.h"

pplx::task<std::shared_ptr<Stormancer::P2PTunnel>> runClient(std::shared_ptr<Stormancer::Client> client, const std::string& ticket, const std::function<void(ConnectionStatus)> &connectionStatusCallback);
pplx::task<void> runServer(
	std::shared_ptr<Stormancer::Client> client,
	std::string connectionToken,
	std::function<void(std::shared_ptr<Stormancer::P2PTunnel>)>  onStartServerReceived,
	std::function<void()> onStopServerReceived);


DedicatedServerModule::DedicatedServerModule(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers):
	_id(id)
{
	//Get connection port passed as environment variable when the Stormancer app starts the server
	size_t len = 256;
	char* buffer = new char[256];
	auto err_no = _dupenv_s(&buffer, &len, "P2Pport");

	Stormancer::IClientFactory::SetConfig(id, [endPoint, maxPeers, accountID, applicationName, err_no, len, buffer]() {
		//std::shared_ptr<Stormancer::ConsoleLogger> logger = std::make_shared<Stormancer::ConsoleLogger>();
		std::shared_ptr<Stormancer::FileLogger> logger = nullptr;
		auto config = Stormancer::Configuration::create(endPoint, accountID, applicationName);
		config->actionDispatcher = std::make_shared<Stormancer::MainThreadActionDispatcher>();		
		config->maxPeers = maxPeers;
		// Check if p2p port is set in var env
		if ((err_no == 0) && (len > 0))
		{
			//Server
			auto port = atoi(buffer);
			config->p2pServerPort = port;	
		}
		else
		{
			//Client
			logger = std::make_shared<Stormancer::FileLogger>("server.txt", false);
		}
		config->logger = logger;
		//Adds the auth plugin to the client. It enable the AuthenticationService to easily interact with the server authentication plugin.
		config->addPlugin(new Stormancer::AuthenticationPlugin());
		config->addPlugin(new Stormancer::GameSessionPluginP2P());
		return config;
	});
}

Task_ptr<Endpoint> DedicatedServerModule::startClient(std::string ticket)
{
	// Lambda callback
	std::function<void(ConnectionStatus)> clientCallback = [this](ConnectionStatus connectionStatus) {
		clientStatus = connectionStatus;
		this->updateConnectionStatus((int)connectionStatus);
	};

	auto client = Stormancer::IClientFactory::GetClient(_id);
	auto logger = client->dependencyResolver()->resolve<Stormancer::ILogger>();
	auto dispatcher = client->dependencyResolver()->resolve<Stormancer::IActionDispatcher>();
	return Stormancer::Task<Endpoint>::create(
		runClient(client, ticket, clientCallback).then([this](std::shared_ptr<Stormancer::P2PTunnel> t) {
		_p2pTunnel = t;
		return Endpoint{ t->ip,t->port };
	}),
		logger,
		dispatcher);
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
