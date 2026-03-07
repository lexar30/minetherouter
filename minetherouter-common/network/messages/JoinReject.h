#pragma once

#include <cstdint>

struct JoinReject
{
    uint16_t reasonCode = 0;
};

inline void Serialize(ByteWriter& w, const JoinReject& msg)
{
    w.writeU16(msg.reasonCode);
}

inline bool Deserialize(ByteReader& r, JoinReject& outMsg)
{
    outMsg.reasonCode = r.readU16();
    return !r.hasError();
}