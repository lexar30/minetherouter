#include <core/ByteWriter.h>

#include "ProtocolContract.h"

void ProtocolContract::SerializeMessage(std::vector<uint8_t>& messageOut, MessageType messageType, const std::vector<uint8_t>& payload, ProtocolStatus& statusOut) const
{
	messageOut.clear();
	statusOut = ProtocolStatus::Undefined;

	if(payload.size() > MAX_PAYLOAD_SIZE) {
		statusOut = ProtocolStatus::TooLarge;
		return;
	}

	messageOut.reserve(HEADER_SIZE + payload.size());

	messageOut.push_back(static_cast<uint16_t>(messageType) & 0xFF);
	messageOut.push_back((static_cast<uint16_t>(messageType) >> 8) & 0xFF);

	const uint32_t payloadSize = static_cast<uint32_t>(payload.size());
	messageOut.push_back(payloadSize & 0xFF);
	messageOut.push_back((payloadSize >> 8) & 0xFF);
	messageOut.push_back((payloadSize >> 16) & 0xFF);
	messageOut.push_back((payloadSize >> 24) & 0xFF);

	messageOut.insert(messageOut.end(), payload.begin(), payload.end());

	statusOut = ProtocolStatus::Success;
}

void ProtocolContract::TryDeserializeMessage(const std::vector<uint8_t>& message, MessageType& outMessageType, std::vector<uint8_t>& outPayload, ProtocolStatus& statusOut) const
{
	outMessageType = MessageType::Undefined;
	outPayload.clear();
	statusOut = ProtocolStatus::Undefined;

	if (message.size() < HEADER_SIZE) {
		statusOut = ProtocolStatus::NotEnoughBytes;
		return;
	}

	outMessageType = static_cast<MessageType>(
		static_cast<uint16_t>(message[0])
		| (static_cast<uint16_t>(message[1]) << 8)
		);

	const uint32_t payloadSize =
		static_cast<uint32_t>(message[2])
		| (static_cast<uint32_t>(message[3]) << 8)
		| (static_cast<uint32_t>(message[4]) << 16)
		| (static_cast<uint32_t>(message[5]) << 24);

	if (static_cast<size_t>(payloadSize) > MAX_PAYLOAD_SIZE) {
		statusOut = ProtocolStatus::TooLarge;
		return;
	}

	const size_t fullMessageSize = HEADER_SIZE + static_cast<size_t>(payloadSize);
	if (fullMessageSize > message.size()) {
		statusOut = ProtocolStatus::NotEnoughBytes;
		return;
	}

	outPayload.insert(outPayload.end(), message.begin() + HEADER_SIZE, message.begin() + HEADER_SIZE + payloadSize);
	statusOut = ProtocolStatus::Success;
}
