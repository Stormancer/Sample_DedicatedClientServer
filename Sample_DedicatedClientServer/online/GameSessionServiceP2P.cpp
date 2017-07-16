#include "stdafx.h"
#include <stormancer.h>
#include "GameSessionServiceP2P.h"


namespace Stormancer
{
	const std::string P2P_SERVER_ID = "IntrepidServer";

	GameSessionServiceP2P::GameSessionServiceP2P(Scene_ptr scene)
	{
		_scene = scene;

		_logger = scene->dependencyResolver()->resolve<ILogger>();

		_scene->addRoute("server.started", [this](Packetisp_ptr packet) {
			this->_waitServerTce.set(packet->readObject<ServerStartedMessage>().p2pToken);
		});
		_scene->addRoute("player.update", [this](Packetisp_ptr packet) {
			auto update = packet->readObject<Stormancer::PlayerUpdate>();
			SessionPlayer player(update.UserId, (PlayerStatus)update.Status);

			auto end = this->_users.end();
			auto it = std::find_if(this->_users.begin(), end, [player](SessionPlayer p) { return p.PlayerId == player.PlayerId; });
			if (it == end)
			{
				this->_users.push_back(player);
			}
			else
			{
				*it = player;
			}
			this->_onConnectedPlayersChanged(SessionPlayerUpdateArg(player, update.Data));
		});

		_scene->addRoute("player.p2ptoken", [this](Packetisp_ptr packet) {
			auto p2pToken = packet->readObject<std::string>();
			_scene->openP2PConnection(p2pToken).then([this](std::shared_ptr<Stormancer::P2PScenePeer> p2pPeer) {
				return p2pPeer->openP2PTunnel(P2P_SERVER_ID).then([this](std::shared_ptr<Stormancer::P2PTunnel> guestTunnel)
				{
					if (_onTunnelOpened)
					{
						_onTunnelOpened(guestTunnel);
					}
				});
			}).then([this](pplx::task<void> t) {
				try
				{
					t.get();
				}
				catch (const std::exception& ex)
				{
					_logger->log(ex);
				}
			});
		});
	}

	pplx::task<std::string> GameSessionServiceP2P::waitServerReady(pplx::cancellation_token token)
	{
		return pplx::create_task(_waitServerTce, pplx::task_options(token));
	}

	std::vector<SessionPlayer> GameSessionServiceP2P::GetConnectedPlayers()
	{
		return this->_users;
	}

	void GameSessionServiceP2P::unsubscribeConnectedPlayersChanged(Action<SessionPlayerUpdateArg>::TIterator handle)
	{
		this->_onConnectedPlayersChanged.erase(handle);
	}

	std::function<void()> Stormancer::GameSessionServiceP2P::OnConnectedPlayerChanged(std::function<void(SessionPlayer, std::string)> callback)
	{
		auto iterator = this->_onConnectedPlayersChanged.push_back([callback](SessionPlayerUpdateArg args) {callback(args.sessionPlayer, args.data); });
		return [iterator, this]() {
			unsubscribeConnectedPlayersChanged(iterator);
		};
	}

	void GameSessionServiceP2P::OnTunnelOpened(std::function<void(std::shared_ptr<Stormancer::P2PTunnel>)> callback)
	{
		_onTunnelOpened = callback;
	}



	pplx::task<void> GameSessionServiceP2P::connect()
	{
		return _scene->connect();
	}

	pplx::task<void> GameSessionServiceP2P::reset()
	{
		auto rpc = _scene->dependencyResolver()->resolve<RpcService>();
		return rpc->rpcWriter("gamesession.reset", [](bytestream* _) {});
	}

	pplx::task<void> GameSessionServiceP2P::disconnect()
	{
		if (_scene)
		{
			return _scene->disconnect();
		}
		else
		{
			return pplx::task_from_result();
		}
	}

	void GameSessionServiceP2P::ready(std::string data)
	{
		_scene->sendPacket("player.ready", [data](Stormancer::bytestream* stream) {
			msgpack::pack(stream, data);
		});
	}

}
