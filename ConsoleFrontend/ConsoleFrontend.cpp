// ConsoleFrontend.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "IClientDCS.h"
#include "IServerDCS.h"
#include <memory>
#include <stdlib.h>  
#include <stdio.h>
#include <thread>
#include "Logger/ILogger.h"
#include "Windows.h"
#include <fstream>
#include <string>
#include <iostream>

using namespace SampleDCS;

int main(int argc, char *argv[])
{		
	size_t len = 256;
	char *buffer = new char[256];
	bool isRunning = true;

	std::string startingMap = "";
	for (int i = 0; i < argc; i++) 
	{
		if (std::string(argv[i]) == "-StartingMap")
		{
			startingMap = argv[i + 1];
			std::cout << "Found starting";
		}
	
	}
	printf("Starting map %s \n", startingMap.c_str());

	//Get connection token passed as environment variable when the Stormancer app starts the server (Plugins/GameSessions/GameSessionService.cs line 413)
	auto err_no = _dupenv_s(&buffer, &len, "connectionToken");

	//auto logger = std::make_shared<Stormancer::ConsoleLogger>();
	//auto config = Stormancer::Configuration::create("http://api.stormancer.com:8081", "samples-jm", "sample-dedicated");
	//config->logger = logger;
	////Adds the auth plugin to the client. It enable the AuthenticationService to easily interact with the server authentication plugin.
	//config->addPlugin(new Stormancer::AuthenticationPlugin());
	//config->addPlugin(new Stormancer::GameSessionPluginP2P());
	//auto client = Stormancer::Client::create(config);
	
	std::function<void(void)> shutdownServer = [&isRunning]() { isRunning = false; };
	std::function<void(Endpoint)> startServer = [](Endpoint e) {};
	
	std::shared_ptr<SampleDCS::IClientDCS> client;
	std::shared_ptr<SampleDCS::IServerDCS> server;
	if (err_no || !len)//Failed to get connection token. We are a client.
	{

		client = IClientDCS::MakeClientDCS(180, "http://127.0.0.1:8081/", "ue4dedicatedserveraccount", "ue4server", 1);		
		client->OnConnectionStatusChange = [](int status) {};

		//Client
		std::string randId = std::to_string(rand() % 1000);
		client->RunClient(randId)->Then([client, startingMap](StormancerResult<void> res)
		{
			client->TravelToMap(startingMap)->Then([client](StormancerResult<Endpoint> e)
			{
				auto logger = Stormancer::ILogger::instance();

				if (e.Success())
				{					
					logger->log(Stormancer::LogLevel::Debug, "ConsoleFrontEnd", "Client connection success", e.Get().host + " " + std::to_string(e.Get().port));
					client->TravelToMap("MapID1");
				}
				else
				{
					logger->log(Stormancer::LogLevel::Debug, "ConsoleFrontEnd", "Client connexion failed : ", e.Reason());
				}
			});
		});
	}
	else
	{
		server = IServerDCS::MakeServerDCS(180, "http://127.0.0.1:8081/", "ue4dedicatedserveraccount", "ue4server", 1000);
		server->RunServer(std::string(buffer), startServer, shutdownServer);
	}
	
	while (isRunning)
	{
		std::this_thread::sleep_for((std::chrono::milliseconds)10);
		if (client == nullptr)
			server->Tick();
		else
			client->Tick();
	}
	return 0;
}
