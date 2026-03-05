#pragma once

#include <cstdint>

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