#pragma once

#include <cstdint>
#include <vector>

enum class MessageType : uint16_t {
	Undefined = 0,
	Ping = 1,
	Pong = 2,
	JoinRequest = 3,
	JoinAccept = 4,
	JoinReject = 5,
	InputCmd = 6,
	WorldState = 7,

	LAST = 8,
};

enum class ProtocolStatus {
	Undefined = 0,
	Success = 1,
	NotEnoughBytes = 2,
	TooLarge = 3,
	BadHeader = 4,

	LAST = 5,
};

static class ProtocolContract
{
public:
	static constexpr uint8_t PROTOCOL_VERSION = 1;
	static constexpr size_t HEADER_SIZE = sizeof(uint16_t) + sizeof(uint32_t);
	static constexpr size_t MAX_PAYLOAD_SIZE = 1024 * 1024; // 1 MB

	void SerializeMessage(std::vector<uint8_t>& messageOut, MessageType messageType, const std::vector<uint8_t>& payload, ProtocolStatus& statusOut) const;
	void TryDeserializeMessage(const std::vector<uint8_t>& message, MessageType& outMessageType, std::vector<uint8_t>& outPayload, ProtocolStatus& statusOut) const;
};


