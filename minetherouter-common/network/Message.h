#pragma once

#include <cstdint>
#include <vector>

#include <network/MessageType.h>

struct Message {
	MessageType type = MessageType::Undefined;
	std::vector<uint8_t> payload;
};
