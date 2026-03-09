#pragma once

#include <cstdint>

#include <network/ByteReader.h>
#include <network/ByteWriter.h>

namespace mtr::network::messages {
    enum class JoinRejectReason : uint8_t {
        UNDEFINED = 0,
        BAD_PROTOCOL_VERSION = 1,
        EMPTY_PLAYER_NAME = 2,
    };

    struct JoinReject
    {
        uint16_t reasonCode = 0;
    };

    inline void Serialize(mtr::network::core::ByteWriter& w, const JoinReject& msg)
    {
        w.writeU16(msg.reasonCode);
    }

    inline bool Deserialize(mtr::network::core::ByteReader& r, JoinReject& outMsg)
    {
        outMsg.reasonCode = r.readU16();
        return !r.hasError();
    }
}