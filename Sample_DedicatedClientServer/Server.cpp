#include "stdafx.h"
#include "online\GameSessionServiceP2P.h"


//Starts a server that connect to the support scene using an opaque connection process passed on server startup
pplx::task<void> runServer(std::shared_ptr<Stormancer::Client> client, std::string connectionToken, bool& isRunning)
{
	auto logger = client->dependencyResolver()->resolve<Stormancer::ILogger>();
	logger->log(Stormancer::LogLevel::Info, "startup", "starting as server");

	

	return client->connectToPrivateScene(connectionToken).then([&isRunning](Stormancer::Scene_ptr scene) {
		auto tunnel = scene->registerP2PServer(Stormancer::P2P_SERVER_ID);
		auto gameSession = scene->dependencyResolver()->resolve<Stormancer::GameSessionServiceP2P>();
		gameSession->OnShutdownReceived([&isRunning]() {
			isRunning = false;
		});
		gameSession->ready("");
		return;
	});
	
	//client->connectToPrivateScene(connectionToken).then([]
	
}
