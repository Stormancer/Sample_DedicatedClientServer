#pragma once

namespace SampleDCS {
	enum class ConnectionStatus
	{
		Authenticating,
		FindLocator,
		ConnectingLocator,
		FindingGameSession,
		ConnectingToGameSession,
		WaitingServer,
		OpeningConnection,
		Connecting,
	};
}