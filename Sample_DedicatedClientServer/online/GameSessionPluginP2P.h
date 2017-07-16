#pragma once
#include <headers.h>
#include <IPlugin.h>



namespace Stormancer
{
class GameSessionPluginP2P : public IPlugin
{
public:
	void sceneCreated(Scene* scene) override;
};
};
