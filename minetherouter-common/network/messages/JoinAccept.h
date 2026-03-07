#pragma once

#include <cstdint>

struct JoinAccept
{
    uint32_t clientId = 0;
};

inline void Serialize(ByteWriter& w, const JoinAccept& msg)
{
    w.writeU32(msg.clientId);
}

inline bool Deserialize(ByteReader& r, JoinAccept& outMsg)
{
    outMsg.clientId = r.readU32();
    return !r.hasError();
}