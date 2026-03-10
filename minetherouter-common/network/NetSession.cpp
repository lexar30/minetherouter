#include "NetSession.h"
#include "ProtocolContract.h"

namespace mtr::common::network {
	void NetSession::PushReceivedBytes(std::span<const uint8_t> bytes)
	{
		if (bytes.empty()) {
			return;
		}

		if (!IsOpen()) {
			return;
		}

		_incomingBytes.insert(_incomingBytes.end(), bytes.begin(), bytes.end());

		TryExtractIncomingMessages();
	}

	std::vector<uint8_t> NetSession::ConsumeOutgoingBytes()
	{
		std::vector<uint8_t> bytes = std::move(_outgoingBytes);
		_outgoingBytes.clear();
		return bytes;
	}

	bool NetSession::QueueOutgoingMessage(MessageType type, const std::vector<uint8_t>& payload)
	{
		// Invalid outgoing message does not change session state and is rejected by returning false.
		if (!IsOpen()) {
			return false;
		}

		std::vector<uint8_t> bytesOut;
		const ProtocolStatus protocolStatus = ProtocolContract::SerializeMessage(bytesOut, type, payload);
		if (protocolStatus != ProtocolStatus::Success) {
			return false;
		}

		_outgoingBytes.insert(_outgoingBytes.end(), bytesOut.begin(), bytesOut.end());
		return true;
	}

	bool NetSession::TryDequeueIncomingMessage(Message& outMessage)
	{
		if (_incomingMessages.empty()) {
			return false;
		}

		outMessage = std::move(_incomingMessages.front());
		_incomingMessages.pop();
		return true;
	}

	void NetSession::TryExtractIncomingMessages()
	{
		// Invalid incoming data puts session into ProtocolError.
		// Incomplete data is kept until more bytes arrive.
		ProtocolStatus protocolStatus = ProtocolStatus::Undefined;

		do {
			size_t readBytesCount = 0;
			MessageType msgType = MessageType::Undefined;
			std::vector<uint8_t> payload;

			protocolStatus = ProtocolContract::TryDeserializeMessage(
				_incomingBytes,
				msgType,
				payload,
				readBytesCount
			);

			if (protocolStatus == ProtocolStatus::NotEnoughBytes) {
				return;
			}

			if (protocolStatus != ProtocolStatus::Success) {
				_state = State::ProtocolError;
				return;
			}

			if (readBytesCount == 0 || readBytesCount > _incomingBytes.size()) {
				_state = State::ProtocolError;
				return;
			}

			_incomingMessages.push({ msgType, std::move(payload) });
			_incomingBytes.erase(_incomingBytes.begin(), _incomingBytes.begin() + readBytesCount);

		} while (protocolStatus == ProtocolStatus::Success);
	}

	void NetSession::Close() {
		_state = State::Closed;
	}
}