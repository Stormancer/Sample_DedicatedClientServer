#pragma once

#include "Authentication/AuthenticationService.h"
#include "online/GameSessionServiceP2P.h"
#include "public/DedicatedServerModule.h"
#include "ClientBase.h"

class Client : public ClientBase
{
public:
	
	Client(size_t clientID, std::shared_ptr<Stormancer::Client> client);


	pplx::task<void> runClient(std::shared_ptr<Stormancer::Client> client, const std::string& ticket, const std::function<void(ConnectionStatus)> &connectionStatusCallback);
	pplx::task<std::shared_ptr<Stormancer::P2PTunnel>> connectToMap(std::string mapId, const std::function<void(ConnectionStatus)> &connectionStatusCallback);
	pplx::task<void> disconnectFromMap();

	size_t _OnlineClientID;
	std::shared_ptr<Stormancer::Client> _client;

private:
	std::string _mapToken;
};