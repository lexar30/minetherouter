#pragma once

#include <cstdint>

struct Ping
{
    uint32_t sequence = 0;
};

inline void Serialize(ByteWriter& w, const Ping& msg)
{
    w.writeU32(msg.sequence);
}

inline bool Deserialize(ByteReader& r, Ping& outMsg)
{
    outMsg.sequence = r.readU32();
    return !r.hasError();
}