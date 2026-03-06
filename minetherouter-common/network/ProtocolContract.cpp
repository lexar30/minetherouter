#include <network/ProtocolContract.h>
#include <network/MessageType.h>
#include <network/ByteReader.h>
#include <network/ByteWriter.h>

ProtocolStatus ProtocolContract::SerializeMessage(std::vector<uint8_t>& bytesOut, MessageType messageType, const std::vector<uint8_t>& payload)
{
	bytesOut.clear();

	if(payload.size() > MAX_PAYLOAD_SIZE) {
		return ProtocolStatus::TooLarge;
	}

	ByteWriter byteWriter;

	bytesOut.reserve(HEADER_SIZE + payload.size());

	byteWriter.writeU16(static_cast<uint16_t>(messageType));

	const uint32_t payloadSize = static_cast<uint32_t>(payload.size());
	byteWriter.writeU32(payloadSize);
	byteWriter.writeBytes(payload.data(), payload.size());

	bytesOut.insert(bytesOut.end(), byteWriter.data(), byteWriter.data() + byteWriter.size());

	return ProtocolStatus::Success;
}

ProtocolStatus ProtocolContract::TryDeserializeMessage(const std::vector<uint8_t>& bytes, MessageType& outMessageType, std::vector<uint8_t>& outPayload, size_t& readBytesCount)
{
	readBytesCount = 0;
	outPayload.clear();
	outMessageType = MessageType::Undefined;

	ByteReader byteReader;
	byteReader.reset(bytes.data(), bytes.size());

	if (byteReader.remaining() < HEADER_SIZE) {
		return ProtocolStatus::NotEnoughBytes;
	}

	outMessageType = static_cast<MessageType>(byteReader.readU16());

	const bool isCorrectType = outMessageType > MessageType::Undefined && outMessageType < MessageType::LAST;
	if(!isCorrectType) {
		return ProtocolStatus::BadHeader;
	}

	const uint32_t payloadSize = byteReader.readU32();

	if (static_cast<size_t>(payloadSize) > MAX_PAYLOAD_SIZE) {
		return ProtocolStatus::TooLarge;
	}

	const size_t plannedReadBytesCount = HEADER_SIZE + static_cast<size_t>(payloadSize);
	if (plannedReadBytesCount > bytes.size()) {
		return ProtocolStatus::NotEnoughBytes;
	}

	outPayload = byteReader.readBytes(payloadSize);
	readBytesCount = plannedReadBytesCount;
	return ProtocolStatus::Success;
}
