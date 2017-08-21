#include "stdafx.h"
#include "IServerDCS.h"
#include "ServerDCS.h"

std::shared_ptr<SampleDCS::IServerDCS> SampleDCS::IServerDCS::MakeServerDCS(size_t id, const std::string & endPoint, std::string accountID, std::string applicationName, int maxPeers)
{
	return std::make_shared<SampleDCS::ServerDCS>(SampleDCS::ServerDCS(id, endPoint, accountID, applicationName, maxPeers));
}
