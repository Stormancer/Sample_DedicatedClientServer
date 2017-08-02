// ConsoleFrontend.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "DedicatedServerModule.h"
#include <memory>
#include <stdlib.h>  
#include <stdio.h>
#include <thread>
#include "Logger/ILogger.h"
#include "Windows.h"
#include <fstream>
#include <string>
#include <iostream>

int main(int argc, char *argv[])
{	
	/*bool waitDebugger = true;
	for (int i = 0; i < argc; i++)
	{
		if (std::string(argv[i]) == "-noDebugger")
		{
			waitDebugger = false;
		}
	}
	while (waitDebugger && !IsDebuggerPresent())
	{
		std::this_thread::sleep_for((std::chrono::milliseconds)10);
	}*/

	//while (!IsDebuggerPresent())
	//{
	//	std::this_thread::sleep_for((std::chrono::milliseconds)10);
	//}
	//DebugBreak();
	
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
	
	std::function<void(void)> shutdownServer = [&isRunning]() { isRunning = false; };
	std::function<void(Endpoint)> startServer = [](Endpoint e) {};
	std::shared_ptr<DedicatedServerModule> onlineModule;
	if (err_no || !len)//Failed to get connection token. We are a client.
	{
		onlineModule = std::make_shared<DedicatedServerModule>(180, "http://127.0.0.1:8081/", "ue4dedicatedserveraccount", "ue4server", 1);
		//Client
		std::string randId = std::to_string(rand() % 1000);
		onlineModule->startClient(randId)->Then([](StormancerResult<Endpoint> e) {
			auto logger = Stormancer::ILogger::instance();
		
			if (e.Success())
			{			
				logger->log(Stormancer::LogLevel::Debug, "ConsoleFrontEnd", "Client connection success", e.Get().host +" " + std::to_string(e.Get().port));
			}
			else
			{
				logger->log(Stormancer::LogLevel::Debug, "ConsoleFrontEnd", "Client connexion failed : ", e.Reason());
			}		
		});

	}
	else
	{
		//DebugBreak();
		onlineModule = std::make_shared<DedicatedServerModule>(180, "http://127.0.0.1:8081/", "ue4dedicatedserveraccount", "ue4server", 1000);
		onlineModule->startServer(std::string(buffer), startServer, shutdownServer);
	}
	
	while (isRunning)
	{
		std::this_thread::sleep_for((std::chrono::milliseconds)10);
		onlineModule->Tick();
	}
	return 0;
}
