#pragma once

#include <cstdint>

namespace mtr::server::network
{
	enum class NetworkSessionState
	{
		AwaitingJoin,
		Joined,
		Rejected
	};

	class NetworkSession
	{
		NetworkSessionState state = NetworkSessionState::AwaitingJoin;
		uint32_t playerId = -1;
	};
}

