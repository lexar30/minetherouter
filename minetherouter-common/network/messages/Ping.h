#pragma once

#include <cstdint>
#include <network/ByteReader.h>
#include <network/ByteWriter.h>

namespace mtr::common::network::messages {
    struct Ping
    {
        uint32_t sequence = 0;
    };

    inline void Serialize(mtr::common::network::core::ByteWriter& w, const Ping& msg)
    {
        w.writeU32(msg.sequence);
    }

    inline bool Deserialize(mtr::common::network::core::ByteReader& r, Ping& outMsg)
    {
        outMsg.sequence = r.readU32();
        return !r.hasError();
    }
}