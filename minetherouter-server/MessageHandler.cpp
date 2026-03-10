#include "MessageHandler.h"
#include "Server.h"
#include <network/Message.h>
#include <utils/Logger.h>
#include <network/messages/JoinRequest.h>
#include <network/MessageCodec.h>

namespace mtr::server::network {

	void MessageHandler::ProcessMessage(Server& server, NetworkSession& session, const mtr::common::network::Message& message)
	{
		switch (message.type)
		{
		case mtr::common::network::MessageType::Ping:
			ProcessPingMessage(server, session, message);
			break;
		case mtr::common::network::MessageType::Pong:
			ProcessPongMessage(server, session, message);
			break;
		case mtr::common::network::MessageType::JoinRequest:
			ProcessJoinRequestMessage(server, session, message);
			break;
		case mtr::common::network::MessageType::JoinAccept:
			ProcessJoinAcceptMessage(server, session, message);
			break;
		case mtr::common::network::MessageType::JoinReject:
			ProcessJoinRejectMessage(server, session, message);
			break;
		default:
			LOG_ERROR("Received unknown message type");
			break;
		}
	}

	void MessageHandler::ProcessPingMessage(Server& server, NetworkSession& session, const mtr::common::network::Message& message)
	{
		LOG_INFO("Received Ping message");
	}

	void MessageHandler::ProcessPongMessage(Server& server, NetworkSession& session, const mtr::common::network::Message& message)
	{
		LOG_INFO("Received Pong message");
	}

	void MessageHandler::ProcessJoinRequestMessage(Server& server, NetworkSession& session, const mtr::common::network::Message& message)
	{
		LOG_INFO("Received Join Request message");

		mtr::common::network::messages::JoinRequest joinRequest;
		const bool success = mtr::common::network::DecodePayload(message, mtr::common::network::MessageType::JoinRequest, joinRequest);
		if (!success) {
			LOG_WARNING("Failed to decode Join Request message");
			return;
		}

		LOG_INFO("Player name: " + joinRequest.playerName);
	}

	void MessageHandler::ProcessJoinAcceptMessage(Server& server, NetworkSession& session, const mtr::common::network::Message& message)
	{
		LOG_INFO("Received Join Accept message");
	}

	void MessageHandler::ProcessJoinRejectMessage(Server& server, NetworkSession& session, const mtr::common::network::Message& message)
	{
		LOG_INFO("Received Join Reject message");
	}
}
