#include "stdafx.h"
#include "GameSessionPluginP2P.h"
#include "GameSessionServiceP2P.h"

namespace Stormancer
{
void GameSessionPluginP2P::sceneCreated(Scene* scene)
{
	if (scene)
	{
		auto name = scene->getHostMetadata("stormancer.gamesession");
		if (name.length() > 0)
		{
			auto service = std::make_shared<GameSessionServiceP2P>(scene->shared_from_this());
			scene->dependencyResolver()->registerDependency<GameSessionServiceP2P>(service);
		}
	}
}
};
