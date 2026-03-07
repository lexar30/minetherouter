#pragma once

#include <cstdint>
#include <string>

struct JoinRequest
{
    uint16_t protocolVersion = 0;
    std::string playerName;
};

inline void Serialize(ByteWriter& w, const JoinRequest& msg)
{
    w.writeU16(msg.protocolVersion);
    w.writeString(msg.playerName);
}

inline bool Deserialize(ByteReader& r, JoinRequest& outMsg)
{
    outMsg.protocolVersion = r.readU16();
    outMsg.playerName = r.readString();
    return !r.hasError();
}