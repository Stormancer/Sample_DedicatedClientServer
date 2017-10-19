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

	std::shared_ptr<SampleDCS::IClientDCS> SampleDCS::IClientDCS::MakeClientDCS(size_t id, const std::string & endPoint, std::string accountID, std::string applicationName, int maxPeers, const std::string& token)
	{
		//auto ptr = new SampleDCS::ClientDCS(id, endPoint, accountID, applicationName, maxPeers);
		//auto sptr = std::shared_ptr<SampleDCS::ClientDCS>(ptr, [](SampleDCS::ClientDCS* ptr2) {
		//	delete ptr2;
		//});
		//return sptr;
		return std::make_shared<SampleDCS::ClientDCS>(SampleDCS::ClientDCS(id, endPoint, accountID, applicationName, maxPeers, token));
	}


}