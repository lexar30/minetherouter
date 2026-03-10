#pragma once

#include <cstdint>
#include <unordered_map>

namespace mtr::common::network {
	class NetworkSession;
}

namespace mtr::server
{
	class Server
	{

	private:
		uint32_t _nextPlayerId = 0;

		std::unordered_map<uint32_t, mtr::common::network::NetworkSession> _sessionsByPlayerId;
	};
}

