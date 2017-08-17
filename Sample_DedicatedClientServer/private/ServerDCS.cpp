#include "stdafx.h"

#include "ServerDCS.h"
#include "Authentication/AuthenticationPlugin.h"
#include "GameSessionPluginP2P.h"
#include "IClientFactory.h"
#include "Compatibility/pplxtasks_noexcept.h"



namespace SampleDCS
{
	ServerDCS::ServerDCS(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers)
	{
		size_t len = 256;
		char* buffer = new char[256];
		auto err_no = _dupenv_s(&buffer, &len, "P2Pport");

		Stormancer::IClientFactory::SetConfig(id, [endPoint, maxPeers, accountID, applicationName, err_no, len, buffer]() {
			//std::shared_ptr<Stormancer::ConsoleLogger> logger = std::make_shared<Stormancer::ConsoleLogger>();

			auto config = Stormancer::Configuration::create(endPoint, accountID, applicationName);
			config->actionDispatcher = std::make_shared<Stormancer::MainThreadActionDispatcher>();
			config->maxPeers = maxPeers;

			//Server
			auto port = atoi(buffer);
			config->p2pServerPort = port;
			std::shared_ptr<Stormancer::FileLogger> logger = nullptr;
			logger = std::make_shared<Stormancer::FileLogger>("D:/Workspace/Sample_DedicatedClientServer/server.txt", false);
			config->logger = logger;

			//Adds the auth plugin to the client. It enable the AuthenticationService to easily interact with the server authentication plugin.
			config->addPlugin(new Stormancer::AuthenticationPlugin());
			config->addPlugin(new Stormancer::GameSessionPluginP2P());
			return config;
		});

		this->Init(id);
	}

	//Starts a server that connect to the support scene using an opaque connection process passed on server startup
	pplx::task<void> ServerDCS::RunServer(
		std::string connectionToken,
		std::function<void(Endpoint)>  onStartServerReceived,
		std::function<void()> onStopServerReceived )
	{
		auto logger = _client->dependencyResolver()->resolve<Stormancer::ILogger>();
		logger->log(Stormancer::LogLevel::Info, "startup", "starting as server");


		return _client->connectToPrivateScene(connectionToken).then([onStopServerReceived, onStartServerReceived, logger, this](Stormancer::Scene_ptr scene) {
			auto tunnel = scene->registerP2PServer(Stormancer::P2P_SERVER_ID);
			auto gameSession = scene->dependencyResolver()->resolve<Stormancer::GameSessionServiceP2P>();

			gameSession->OnShutdownReceived([onStopServerReceived, onStartServerReceived, logger]() {
				if (onStopServerReceived)
				{
					onStopServerReceived();
				}
				else
				{
					logger->log(Stormancer::LogLevel::Info, "Error", "OnStopServerReceived callback failed");
				}
			});

			if (onStartServerReceived)
			{
				Endpoint endpoint;
				endpoint.host = tunnel->ip;
				endpoint.port = tunnel->port;
				_p2pTunnel = tunnel;
				onStartServerReceived(endpoint);
			}
			gameSession->ready("");
			return;
		});
	}
}