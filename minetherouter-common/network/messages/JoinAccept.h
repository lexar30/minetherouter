#pragma once

#include <cstdint>

#include <network/ByteReader.h>
#include <network/ByteWriter.h>

namespace mtr::common::network::messages {
    struct JoinAccept
    {
        uint32_t clientId = 0;
    };

    inline void Serialize(mtr::common::network::core::ByteWriter& w, const JoinAccept& msg)
    {
        w.writeU32(msg.clientId);
    }

    inline bool Deserialize(mtr::common::network::core::ByteReader& r, JoinAccept& outMsg)
    {
        outMsg.clientId = r.readU32();
        return !r.hasError();
    }
}