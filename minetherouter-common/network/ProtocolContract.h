#pragma once

#include <cstdint>
#include <vector>

namespace mtr::common::network {
	enum class ProtocolStatus {
		Undefined = 0,
		Success = 1,
		NotEnoughBytes = 2,
		TooLarge = 3,
		BadHeader = 4,

		LAST = 5,
	};

	enum class MessageType : uint16_t;

	class ProtocolContract
	{
	public:
		static constexpr uint8_t PROTOCOL_VERSION = 1;
		static constexpr size_t HEADER_SIZE = sizeof(uint16_t) + sizeof(uint32_t);
		static constexpr size_t MAX_PAYLOAD_SIZE = 1024 * 1024; // 1 MB

		static ProtocolStatus SerializeMessage(std::vector<uint8_t>& bytesOut, MessageType messageType, const std::vector<uint8_t>& payload);
		static ProtocolStatus TryDeserializeMessage(const std::vector<uint8_t>& bytes, MessageType& outMessageType, std::vector<uint8_t>& outPayload, size_t& readBytesCount);
	};
}