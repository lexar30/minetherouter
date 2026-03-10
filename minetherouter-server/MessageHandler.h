#pragma once

#include "NetworkSession.h"

namespace mtr::common::network
{
	class Message;
}

namespace mtr::server::network
{
	class Server;

	class MessageHandler
	{
	public:
		void ProcessMessage(Server& server, NetworkSession& session, const mtr::common::network::Message& message);

	private:
		void ProcessPingMessage(Server& server, NetworkSession& session, const mtr::common::network::Message& message);
		void ProcessPongMessage(Server& server, NetworkSession& session, const mtr::common::network::Message& message);
		void ProcessJoinRequestMessage(Server& server, NetworkSession& session, const mtr::common::network::Message& message);
		void ProcessJoinAcceptMessage(Server& server, NetworkSession& session, const mtr::common::network::Message& message);
		void ProcessJoinRejectMessage(Server& server, NetworkSession& session, const mtr::common::network::Message& message);
	};
}