// Sample_05_DedicatedClientServer.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"

#include "online\GameSessionPluginP2P.h"
#include "AuthenticationPlugin.h"

pplx::task<void> runClient(std::shared_ptr<Stormancer::Client> client, std::string ticket);
pplx::task<void> runServer(std::shared_ptr<Stormancer::Client> client, std::string connectionToken,bool& isRunning);




int main()
{
	size_t len = 256;
	char *buffer = new char[256];
	bool isRunning = true;

	//Get connection token passed as environment variable when the Stormancer app starts the server (Plugins/GameSessions/GameSessionService.cs line 413)
	auto err_no = _dupenv_s(&buffer,&len,"connectionToken");
	
	auto logger = std::make_shared<Stormancer::ConsoleLogger>();
	auto config = Stormancer::Configuration::create("http://api.stormancer.com:8081", "samples-jm", "sample-dedicated");
	config->logger = logger;
	//Adds the auth plugin to the client. It enable the AuthenticationService to easily interact with the server authentication plugin.
	config->addPlugin(new Stormancer::AuthenticationPlugin());
	config->addPlugin(new Stormancer::GameSessionPluginP2P());
	auto client = Stormancer::Client::create(config);
	
	if (err_no || !len)//Failed to get connection token. We are a client.
	{
		
		//Client
		runClient(client,"test-ticket").wait();
	}
	else
	{
		
		runServer(client,std::string(buffer),isRunning).wait();
	}

	while (isRunning)
	{
		std::this_thread::sleep_for((std::chrono::milliseconds)10);
	}
    return 0;
}



