#include "stdafx.h"

#include "online/GameSessionServiceP2P.h"
#include "AuthenticationService.h"

pplx::task<void> runClient(std::shared_ptr<Stormancer::Client> client, std::string ticket)
{
	auto logger = client->dependencyResolver()->resolve<Stormancer::ILogger>();
	logger->log(Stormancer::LogLevel::Info, "startup", "starting as client");

	

	auto auth = client->dependencyResolver()->resolve<Stormancer::AuthenticationService>();
	//Login using the steam auth provider (if configured as "mockup", it accepts any ticket as genuine)
	return auth->loginSteam(ticket).then([auth]()
	{
		//Authorize access to the locator private scene and return it.
		return auth->getPrivateScene("locator");

	}).then([](Stormancer::Scene_ptr locator)
	{
		return locator->connect().then([locator] {return locator; });

	}).then([](Stormancer::Scene_ptr locator)
	{
		return locator->dependencyResolver()->resolve<Stormancer::RpcService>()->rpc<std::string, bool>("locator.getshard", true);

	}).then([client](std::string token)
	{
		return client->connectToPrivateScene(token);

	}).then([logger](Stormancer::Scene_ptr gameSessionScene) 
	{
		auto cts = pplx::cancellation_token_source();
		auto gameSession = gameSessionScene->dependencyResolver()->resolve<Stormancer::GameSessionServiceP2P>();
		gameSession->ready("");
		logger->log(Stormancer::LogLevel::Info, "startup", "waiting for server...");
		return gameSession->waitServerReady(cts.get_token())
			.then([gameSessionScene](std::string p2pToken) {
			return std::make_tuple(gameSessionScene, p2pToken);
		});

	}).then([logger](std::tuple<Stormancer::Scene_ptr, std::string> tuple) 
	{
		auto gameSessionScene = std::get<0>(tuple);
		auto p2pToken = std::get<1>(tuple);
		logger->log(Stormancer::LogLevel::Info, "startup", "testing connectivity with dedicated server");
		return gameSessionScene->openP2PConnection(p2pToken);

	}).then([logger](std::shared_ptr<Stormancer::P2PScenePeer> peer)
	{
		logger->log(Stormancer::LogLevel::Info, "startup", "obtaining connection information to dedicated server");
		return peer->openP2PTunnel(Stormancer::P2P_SERVER_ID);// 

	}).then([logger](pplx::task<std::shared_ptr<Stormancer::P2PTunnel>> t) 
	{
		try
		{
			t.get();
			logger->log(Stormancer::LogLevel::Info, "startup", "successful");
		}
		catch (std::exception& ex)
		{
			auto msg = ex.what();
			logger->log(Stormancer::LogLevel::Error, "startup", "failed : " + std::string(msg));
		}
		//Connect unreal!
		
		
	});
	



	
}