#pragma once

#include <cstdint>

struct Pong
{
    uint32_t sequence = 0;
};

inline void Serialize(ByteWriter& w, const Pong& msg)
{
    w.writeU32(msg.sequence);
}

inline bool Deserialize(ByteReader& r, Pong& outMsg)
{
    outMsg.sequence = r.readU32();
    return !r.hasError();
}