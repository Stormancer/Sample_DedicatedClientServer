#include "stdafx.h"
#include "IClientDCS.h"
#include "Authentication/AuthenticationService.h"
#include "Authentication/AuthenticationPlugin.h"
#include "GameSessionServiceP2P.h"
#include "GameSessionPluginP2P.h"
#include "IClientFactory.h"
#include "Compatibility/pplxtasks_noexcept.h"
#include "ClientDCS.h"

namespace SampleDCS {

	ClientDCS::ClientDCS(size_t id, const std::string& endPoint, std::string accountID, std::string applicationName, int maxPeers)
	{
		Stormancer::IClientFactory::SetConfig(id, [endPoint, maxPeers, accountID, applicationName]() {
			//std::shared_ptr<Stormancer::ConsoleLogger> logger = std::make_shared<Stormancer::ConsoleLogger>();

			auto config = Stormancer::Configuration::create(endPoint, accountID, applicationName);
			config->actionDispatcher = std::make_shared<Stormancer::MainThreadActionDispatcher>();
			config->maxPeers = maxPeers;
			std::shared_ptr<Stormancer::ConsoleLogger> logger = nullptr;
			//logger = std::make_shared<Stormancer::FileLogger>("server.txt", false);
			logger = std::make_shared<Stormancer::ConsoleLogger>();
			config->logger = logger;
			//Adds the auth plugin to the client. It enable the AuthenticationService to easily interact with the server authentication plugin.
			config->addPlugin(new Stormancer::AuthenticationPlugin());
			config->addPlugin(new Stormancer::GameSessionPluginP2P());
			return config;
		});

		_stormancerClient = Stormancer::IClientFactory::GetClient(id);
		_logger = _stormancerClient->dependencyResolver()->resolve<Stormancer::ILogger>();
		_actionDispatcher = _stormancerClient->dependencyResolver()->resolve<Stormancer::IActionDispatcher>();
	}

	Task_ptr<void> ClientDCS::RunClient(std::string& ticket)
	{
		return Stormancer::Task<void>::create(_RunClient(ticket), _logger, _actionDispatcher);
	}

	Task_ptr<Endpoint> ClientDCS::TravelToMap(std::string mapID)
	{
		return Stormancer::Task<Endpoint>::create(_TravelToMap(mapID), _logger, _actionDispatcher);
	}

	void ClientDCS::Tick()
	{
		auto dispatcher = std::dynamic_pointer_cast<Stormancer::MainThreadActionDispatcher>(_actionDispatcher);
		dispatcher->update((std::chrono::milliseconds)10);
	}

	pplx::task<void> ClientDCS::_RunClient(std::string& ticket)
	{
		_logger->log(Stormancer::LogLevel::Info, "startup", "starting as client");

		//Try to authenticate to stormancer app
		OnConnectionStatusChange((int)ConnectionStatus::Authenticating);

		auto auth = _stormancerClient->dependencyResolver()->resolve<Stormancer::AuthenticationService>();
		//Login using the steam auth provider (if configured as "mockup", it accepts any ticket as genuine)
		return auth->login(std::map<std::string, std::string>{ {"provider", "test"}, { "login", ticket } }).then([auth, this]()
		{
			//Find locator
			OnConnectionStatusChange((int)ConnectionStatus::FindLocator);
			//Authorize access to the locator private scene and return it.
			return auth->getPrivateScene("locator");

		}).then([this](Stormancer::Scene_ptr locator)
		{
			OnConnectionStatusChange((int)ConnectionStatus::ConnectingLocator);
			// Connecting to locator scene
			return locator->connect();
		});
	}

	pplx::task<Endpoint> ClientDCS::_ConnectToMap(std::string mapID)
	{
		/**
		Throw errors if the client doesn't launch run client
		*/
		auto auth = _stormancerClient->dependencyResolver()->resolve<Stormancer::AuthenticationService>();

		auto result = auth->getPrivateScene("locator").then([this, mapID](Stormancer::Scene_ptr locator)
		{
			// Get Game Session
			_logger->log(Stormancer::LogLevel::Info, "startup", "Finding game session");
			OnConnectionStatusChange((int)ConnectionStatus::FindingGameSession);
			return locator->dependencyResolver()->resolve<Stormancer::RpcService>()->rpc<std::string, std::string>("locator.getshard", mapID);

		}).then([this](std::string token)
		{
			// Connect to Game Session
			_logger->log(Stormancer::LogLevel::Info, "startup", "ConnectingToGameSession");
			OnConnectionStatusChange((int)ConnectionStatus::ConnectingToGameSession);
			return _stormancerClient->connectToPrivateScene(token);

		}).then([this](Stormancer::Scene_ptr gameSessionScene)
		{
			// Wait server ready
			OnConnectionStatusChange((int)ConnectionStatus::WaitingServer);
			auto cts = pplx::cancellation_token_source();
			auto gameSession = gameSessionScene->dependencyResolver()->resolve<Stormancer::GameSessionServiceP2P>();
			gameSession->ready("");
			_logger->log(Stormancer::LogLevel::Info, "startup", "waiting for server...");
			return gameSession->waitServerReady(cts.get_token())
				.then([gameSessionScene](std::string p2pToken) {
				return std::make_tuple(gameSessionScene, p2pToken);
			});

		}).then([this](std::tuple<Stormancer::Scene_ptr, std::string> tuple)
		{
			// Open P2P connection
			OnConnectionStatusChange((int)ConnectionStatus::OpeningConnection);
			auto gameSessionScene = std::get<0>(tuple);
			currentMap = gameSessionScene;
			auto p2pToken = std::get<1>(tuple);
			_logger->log(Stormancer::LogLevel::Info, "startup", "testing connectivity with dedicated server");
			return gameSessionScene->openP2PConnection(p2pToken);

		}).then([this](std::shared_ptr<Stormancer::P2PScenePeer> peer)
		{
			// Open P2P tunnel
			OnConnectionStatusChange((int)ConnectionStatus::Connecting);
			_logger->log(Stormancer::LogLevel::Info, "startup", "obtaining connection information to dedicated server");
			return peer->openP2PTunnel(Stormancer::P2P_SERVER_ID);// 
		}).then([this, mapID](pplx::task<std::shared_ptr<Stormancer::P2PTunnel>> task)
		{	
			try
			{
				auto t = task.get();
				_p2pTunnel = t;
				return pplx::task_from_result(Endpoint{ t->ip,t->port });
			}
			catch (const std::exception& ex)
			{
				_logger->log(Stormancer::LogLevel::Error, "Error", ex.what());
				return pplx::task_from_result(Endpoint{ "",0 });
			}
		});

		return result;	
	}

	pplx::task<void> ClientDCS::_DisconnectFromMap(std::string mapID)
	{
		auto auth = _stormancerClient->dependencyResolver()->resolve<Stormancer::AuthenticationService>();
		_logger->log(Stormancer::LogLevel::Info, "Info", "Disconnecting from map");
		if (currentMap)
		{
			_logger->log(Stormancer::LogLevel::Info, "Info", "Disconnected from map: ", mapID);
			return currentMap->disconnect();
		}
	
		return pplx::task_from_result();
	}

	

	pplx::task<SampleDCS::Endpoint> ClientDCS::_TravelToMap(std::string mapID)
	{
		_logger->log(Stormancer::LogLevel::Debug, "Travel", "Start travel");
		auto result = _DisconnectFromMap(mapID).then([this, mapID]()
			{
				_logger->log(Stormancer::LogLevel::Debug, "Travel", "Connect to map");
				return _ConnectToMap(mapID);
			});

		return result;
		//return _ConnectToMap(mapID);
	}

	

}