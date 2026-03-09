#pragma once

#include <cstdint>
#include <vector>

#include "MessageType.h"

namespace mtr::network {
	struct Message {
		MessageType type;
		std::vector<uint8_t> payload;
	};
}