#include "NetSession.h"
#include "ProtocolContract.h"

#include <MessageType.h>

void NetSession::AcceptBytes(uint8_t* incomingBytes, size_t size)
{
	const bool isCorrectState = _state != NetSessionState::Undefined
		&& _state != NetSessionState::ProtocolError
		&& _state != NetSessionState::Closed;
	if (!isCorrectState) {
		return;
	}

	if (size == 0) {
		return;
	}

	_incoming.insert(_incoming.end(), incomingBytes, incomingBytes + size);

	ProtocolStatus protocolStatus = ProtocolStatus::Undefined;

	do {
		size_t readBytesCount = 0;
		MessageType msgType = MessageType::Undefined;
		std::vector<uint8_t> payload;

		protocolStatus = ProtocolContract::TryDeserializeMessage(
			_incoming,
			msgType,
			payload,
			readBytesCount
		);

		if (protocolStatus == ProtocolStatus::NotEnoughBytes) {
			break;
		}

		if (protocolStatus != ProtocolStatus::Success) {
			_state = NetSessionState::ProtocolError;
			break;
		}

		if (readBytesCount == 0 || readBytesCount > _incoming.size()) {
			_state = NetSessionState::ProtocolError;
			return;
		}

		_unprocessedMessagesIn.push({ msgType, std::move(payload) });
		_incoming.erase(_incoming.begin(), _incoming.begin() + readBytesCount);

	} while (protocolStatus == ProtocolStatus::Success);


}

void NetSession::EnqueueMessage(MessageType type, const std::vector<uint8_t>& payload)
{
	const bool isCorrectState = _state != NetSessionState::Undefined
		&& _state != NetSessionState::ProtocolError
		&& _state != NetSessionState::Closed;
	if (!isCorrectState) {
		return;
	}

	const bool isCorrectMsgType = type > MessageType::Undefined && type < MessageType::LAST;
	if (!isCorrectMsgType) {
		_state == NetSessionState::ProtocolError;
		return;
	}


	//std::vector<uint8_t>& bytesOut, MessageType messageType, const std::vector<uint8_t>& payload)
	std::vector<uint8_t> bytesOut;


	if(bytesOut.size() > ProtocolContract::MAX_PAYLOAD_SIZE || bytesOut.empty()) {
		_state = NetSessionState::ProtocolError;
		return;
	}

	_outcoming.insert(_outcoming.end(), bytesOut.data(), bytesOut.data() + bytesOut.size());
}

void NetSession::ProcessMessages() {
	Message msg;
	while (TryPopMessage(msg)) {
		ProcessMessage(msg);
	}
}

bool NetSession::TryPopMessage(Message& outMessage)
{
	if (_unprocessedMessages.empty()) {
		return false;
	}

	outMessage = std::move(_unprocessedMessages.front());
	_unprocessedMessages.pop();
	return true;
}

void NetSession::ProcessMessage(const Message& msg)
{
	switch (msg.type)
	{
	case MessageType::Undefined:
		// Handle Undefined message
		break;
	case MessageType::Ping:
		// Handle Ping message
		break;
	case MessageType::Pong:
		// Handle Pong message
		break;
	case MessageType::JoinRequest:
		// Handle JoinRequest message
		break;
	case MessageType::JoinAccept:
		// Handle JoinAccept message
		break;
	case MessageType::JoinReject:
		// Handle JoinReject message
		break;
	case MessageType::InputCmd:
		// Handle InputCmd message
		break;
	case MessageType::WorldState:
		// Handle WorldState message
		break;

	case MessageType::LAST:
	default:
		break;
	}

}
