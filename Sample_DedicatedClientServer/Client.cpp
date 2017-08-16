#include "stdafx.h"

#include "Client.h"
#include "Authentication\AuthenticationPlugin.h"
#include "online\GameSessionPluginP2P.h"
#include "IClientFactory.h"

//#include "Public/DedicatedServerModule.h"
/**
Connexion de base (avec continuation)
	1.On commence le programme via un run client
	2.On on connect to map

Changement de mapt (avec continuation)
	1.On delete le tunnel
	2.On disconnect from map
	3.On Connect to map
*/

Client::Client(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers)
{
	//Get connection port passed as environment variable when the Stormancer app starts the server
	size_t len = 256;
	char* buffer = new char[256];
	auto err_no = _dupenv_s(&buffer, &len, "P2Pport");

	Stormancer::IClientFactory::SetConfig(id, [endPoint, maxPeers, accountID, applicationName, err_no, len, buffer]() {
		//std::shared_ptr<Stormancer::ConsoleLogger> logger = std::make_shared<Stormancer::ConsoleLogger>();

		auto config = Stormancer::Configuration::create(endPoint, accountID, applicationName);
		config->actionDispatcher = std::make_shared<Stormancer::MainThreadActionDispatcher>();
		config->maxPeers = maxPeers;
		// Check if p2p port is set in var env
		if ((err_no == 0) && (len > 0))
		{
			//Server
			auto port = atoi(buffer);
			config->p2pServerPort = port;
			std::shared_ptr<Stormancer::FileLogger> logger = nullptr;
			logger = std::make_shared<Stormancer::FileLogger>("D:/Workspace/Sample_DedicatedClientServer/server.txt", false);
			config->logger = logger;
		}
		else
		{
			std::shared_ptr<Stormancer::ConsoleLogger> logger = nullptr;
			//Client
			//logger = std::make_shared<Stormancer::FileLogger>("server.txt", false);
			logger = std::make_shared<Stormancer::ConsoleLogger>();
			config->logger = logger;
		}

		//Adds the auth plugin to the client. It enable the AuthenticationService to easily interact with the server authentication plugin.
		config->addPlugin(new Stormancer::AuthenticationPlugin());
		config->addPlugin(new Stormancer::GameSessionPluginP2P());
		return config;
	});

	_client = Stormancer::IClientFactory::GetClient(id);

	// Lambda callback
	/*clientCallback = [this](ConnectionStatus connectionStatus) {
		clientStatus = connectionStatus;
		this->updateConnectionStatus((int)connectionStatus);
	};

	_onlineClient = std::make_shared<OnlineClient>();

	
	auto logger = client->dependencyResolver()->resolve<Stormancer::ILogger>();
	auto dispatcher = client->dependencyResolver()->resolve<Stormancer::IActionDispatcher>();

	_onlineClient->runClient(client, ticket, clientCallback).then([this, targetMap, clientCallback, logger, dispatcher]() {
		return Stormancer::Task<Endpoint>::create(
			_onlineClient->connectToMap(targetMap, clientCallback).then([this](std::shared_ptr<Stormancer::P2PTunnel> t) {
			_p2pTunnel = t;
			return Endpoint{ t->ip,t->port };
		}),
			logger,
			dispatcher);
	});*/
}

pplx::task<void> Client::runClient(std::shared_ptr<Stormancer::Client> client, const std::string& ticket, std::function<void(ConnectionStatus)> connectionStatusCallback)
{
	auto logger = client->dependencyResolver()->resolve<Stormancer::ILogger>();
	logger->log(Stormancer::LogLevel::Info, "startup", "starting as client");

	//Try to authenticate to stormmancer app
	connectionStatusCallback(ConnectionStatus::Authenticating);

	auto auth = client->dependencyResolver()->resolve<Stormancer::AuthenticationService>();
	//Login using the steam auth provider (if configured as "mockup", it accepts any ticket as genuine)
	return auth->login(std::map<std::string, std::string>{ {"provider", "test"}, { "login",ticket } }).then([auth, connectionStatusCallback]()
	{
		//Find locator
		connectionStatusCallback(ConnectionStatus::FindLocator);
		//Authorize access to the locator private scene and return it.
		return auth->getPrivateScene("locator");

	}).then([connectionStatusCallback](Stormancer::Scene_ptr locator)
	{
		connectionStatusCallback(ConnectionStatus::ConnectingLocator);
		// Connecting to locator scene
		return locator->connect();
	});

}

pplx::task<void> Client::disconnectFromMap()
{
	_client->getPublicScene(_mapToken).then([this](Stormancer::Scene_ptr map)
	{
		return map->disconnect();
	});
}

pplx::task<std::shared_ptr<Stormancer::P2PTunnel>> Client::connectToMap(std::string mapId, std::function<void(ConnectionStatus)> connectionStatusCallback)
{
	_client->getPrivateScene("locator").then([connectionStatusCallback, mapId](Stormancer::Scene_ptr locator)
	{
		// 
		// Get Game Session
		connectionStatusCallback(ConnectionStatus::FindingGameSession);
		return locator->dependencyResolver()->resolve<Stormancer::RpcService>()->rpc<std::string, std::string>("locator.getshard", mapId);

	}).then([this, connectionStatusCallback](std::string token)
	{
		_mapToken = token;
		// Connect to Game Session
		connectionStatusCallback(ConnectionStatus::ConnectingToGameSession);
		return _client->connectToPrivateScene(token);

	}).then([this, connectionStatusCallback](Stormancer::Scene_ptr gameSessionScene)
	{
		// Wait server ready
		connectionStatusCallback(ConnectionStatus::WaitingServer);
		auto cts = pplx::cancellation_token_source();
		auto gameSession = gameSessionScene->dependencyResolver()->resolve<Stormancer::GameSessionServiceP2P>();
		gameSession->ready("");
		_client->logger()->log(Stormancer::LogLevel::Info, "startup", "waiting for server...");		
		return gameSession->waitServerReady(cts.get_token())
			.then([gameSessionScene](std::string p2pToken) {
			return std::make_tuple(gameSessionScene, p2pToken);
		});

	}).then([this, connectionStatusCallback](std::tuple<Stormancer::Scene_ptr, std::string> tuple)
	{
		// Open P2P connection
		connectionStatusCallback(ConnectionStatus::OpeningConnection);
		auto gameSessionScene = std::get<0>(tuple);
		auto p2pToken = std::get<1>(tuple);
		_client->logger()->log(Stormancer::LogLevel::Info, "startup", "testing connectivity with dedicated server");
		return gameSessionScene->openP2PConnection(p2pToken);

	}).then([this, connectionStatusCallback](std::shared_ptr<Stormancer::P2PScenePeer> peer)
	{
		// Open P2P tunnel
		connectionStatusCallback(ConnectionStatus::Connecting);
		_client->logger()->log(Stormancer::LogLevel::Info, "startup", "obtaining connection information to dedicated server");
		return peer->openP2PTunnel(Stormancer::P2P_SERVER_ID);// 
	});	
}
