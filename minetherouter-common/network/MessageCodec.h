#pragma once

#include <vector>
#include <cstdint>

#include <network/ByteWriter.h>
#include <network/ByteReader.h>
#include <network/Message.h>
#include <network/MessageType.h>

template<typename T>
std::vector<uint8_t> EncodePayload(const T& data) {
    ByteWriter w;
    Serialize(w, data);

    if (w.hasError()) {
        return {};
    }

    return std::vector<uint8_t>(w.data(), w.data() + w.size());
}

template<typename T>
bool DecodePayload(const Message& rawMessage, MessageType expectedType, T& outData)
{
    if (rawMessage.type != expectedType) {
        return false;
    }

    ByteReader r;
    r.reset(rawMessage.payload.data(), rawMessage.payload.size());

    if (!Deserialize(r, outData)) {
        return false;
    }

    return r.remaining() == 0 && !r.hasError();
}