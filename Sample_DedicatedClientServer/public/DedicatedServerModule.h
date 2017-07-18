#pragma once
#include "Compatibility\IStormancerTask.h"


struct Endpoint
{
	std::string host;
	unsigned short port;
};
class DedicatedServerModule
{
public:
	DedicatedServerModule(size_t id);
	Task_ptr<Endpoint> startClient(std::string ticket);

	Task_ptr<void> startServer(std::string connectionToken, std::function<void(Endpoint)> startServerCallback, std::function<void()> stopServerCallback);

	void Tick();
private:
	size_t _id;
};