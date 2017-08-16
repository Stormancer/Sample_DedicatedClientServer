#include "stdafx.h"

#include "OnlineClient.h"

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
