#include "stdafx.h"
#include "online\GameSessionServiceP2P.h"


//Starts a server that connect to the support scene using an opaque connection process passed on server startup
pplx::task<void> runServer(
	std::shared_ptr<Stormancer::Client> client,
	std::string connectionToken,
	std::function<void(std::shared_ptr<Stormancer::P2PTunnel>) >  onStartServerReceived,
	std::function<void()> onStopServerReceived )
{
	auto logger = client->dependencyResolver()->resolve<Stormancer::ILogger>();
	logger->log(Stormancer::LogLevel::Info, "startup", "starting as server");

	

	return client->connectToPrivateScene(connectionToken).then([onStopServerReceived, onStartServerReceived](Stormancer::Scene_ptr scene) {
		auto tunnel = scene->registerP2PServer(Stormancer::P2P_SERVER_ID);
		auto gameSession = scene->dependencyResolver()->resolve<Stormancer::GameSessionServiceP2P>();
		gameSession->OnShutdownReceived([onStopServerReceived, onStartServerReceived]() {
			if (onStopServerReceived)
			{
				onStopServerReceived();
			}
			else
			{

			}
		});
		if (onStartServerReceived)
		{
			onStartServerReceived(tunnel);
		}
		gameSession->ready("");
		return;
	});
	
	//client->connectToPrivateScene(connectionToken).then([]
	
}
