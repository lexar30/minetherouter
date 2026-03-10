#pragma once

#include <cstdint>
#include <string>

#include <network/ByteReader.h>
#include <network/ByteWriter.h>

namespace mtr::common::network::messages {
    struct JoinRequest
    {
        uint16_t protocolVersion = 0;
        std::string playerName;
    };

    inline void Serialize(mtr::common::network::core::ByteWriter& w, const JoinRequest& msg)
    {
        w.writeU16(msg.protocolVersion);
        w.writeString(msg.playerName);
    }

    inline bool Deserialize(mtr::common::network::core::ByteReader& r, JoinRequest& outMsg)
    {
        outMsg.protocolVersion = r.readU16();
        outMsg.playerName = r.readString();
        return !r.hasError();
    }
}