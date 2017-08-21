#include "stdafx.h"

#include "ClientDCS.h"
#include "Authentication/AuthenticationPlugin.h"
#include "Authentication/AuthenticationService.h"
#include "GameSessionServiceP2P.h"
#include "GameSessionPluginP2P.h"
#include "IClientFactory.h"
#include "stormancer.h"
#include "Compatibility/pplxtasks_noexcept.h"
#include "ClientDCS.h"

using namespace Stormancer;

namespace SampleDCS {

	std::shared_ptr<SampleDCS::IClientDCS> SampleDCS::IClientDCS::MakeClientDCS(size_t id, const std::string & endPoint, std::string accountID, std::string applicationName, int maxPeers)
	{
		return std::make_shared<SampleDCS::ClientDCS>(SampleDCS::ClientDCS(id, endPoint, accountID, applicationName, maxPeers));
	}


}