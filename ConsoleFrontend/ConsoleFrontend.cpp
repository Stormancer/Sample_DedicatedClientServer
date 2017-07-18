// ConsoleFrontend.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "DedicatedServerModule.h"
#include <memory>
#include  <stdlib.h>  
#include <stdio.h>
#include <thread>

int main()
{
	size_t len = 256;
	char *buffer = new char[256];
	bool isRunning = true;

	//Get connection token passed as environment variable when the Stormancer app starts the server (Plugins/GameSessions/GameSessionService.cs line 413)
	auto err_no = _dupenv_s(&buffer, &len, "connectionToken");

	//auto logger = std::make_shared<Stormancer::ConsoleLogger>();
	//auto config = Stormancer::Configuration::create("http://api.stormancer.com:8081", "samples-jm", "sample-dedicated");
	//config->logger = logger;
	////Adds the auth plugin to the client. It enable the AuthenticationService to easily interact with the server authentication plugin.
	//config->addPlugin(new Stormancer::AuthenticationPlugin());
	//config->addPlugin(new Stormancer::GameSessionPluginP2P());
	//auto client = Stormancer::Client::create(config);
	auto onlineModule = std::make_shared<DedicatedServerModule>(0);
	std::function<void(void)> shutdownServer = [&isRunning]() { isRunning = false; };
	std::function<void(Endpoint)> startServer = [](Endpoint e) {};
	if (err_no || !len)//Failed to get connection token. We are a client.
	{

		//Client
		onlineModule->startClient("test2");

	}
	else
	{

		onlineModule->startServer(std::string(buffer), startServer, shutdownServer);
	}

	while (isRunning)
	{
		std::this_thread::sleep_for((std::chrono::milliseconds)10);
		onlineModule->Tick();
	}
	return 0;
}
