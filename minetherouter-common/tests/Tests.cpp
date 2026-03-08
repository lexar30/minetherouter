#include "Tests.h"

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

#include <network/ByteReader.h>
#include <network/ByteWriter.h>
#include <network/NetSession.h>
#include <network/ProtocolContract.h>
#include <network/Message.h>
#include <network/MessageType.h>

#include <network/messages/Ping.h>
#include <network/messages/Pong.h>
#include <network/messages/JoinRequest.h>
#include <network/messages/JoinAccept.h>
#include <network/messages/JoinReject.h>

#include <network/MessageCodec.h>

namespace Tests {

    namespace ByteReaderWriter {

        void ByteWriter_BasicWriteAndRewrite() {
            ByteWriter w;

            // Write primitive types
            w.writeU8(0xAB);
            w.writeU16(0xCDEF);
            w.writeU32(0x12345678u);
            w.writeI32(-123456);
            w.writeF32(3.5f);

            // Raw bytes
            const uint8_t raw[] = { 0x00, 0xFF, 0x10, 0x20 };
            w.writeBytes(raw, sizeof(raw));

            // String (length-prefixed U32)
            w.writeString("hello");

            ASSERT_EQ(w.hasError(), false);
            ASSERT_EQ(w.size() > 0, true);
            ASSERT_EQ(w.data() == nullptr, false);

            // Rewrite U16 at offset 1 (immediately after first U8)
            w.rewriteU16(1, 0xBEEF);
            ASSERT_EQ(w.hasError(), false);

            // Rewrite U32 at offset 3 (U8 + U16)
            w.rewriteU32(3, 0xDEADBEEFu);
            ASSERT_EQ(w.hasError(), false);

            // Invalid rewrites should set error
            ByteWriter w2;
            w2.writeU8(0x01);
            w2.rewriteU16(0, 0x2222); // buffer too small for U16 rewrite
            ASSERT_EQ(w2.hasError(), true);

            ByteWriter w3;
            w3.writeU32(0x01020304u);
            w3.rewriteU32(2, 0x0); // offset out of bounds for U32 rewrite
            ASSERT_EQ(w3.hasError(), true);
        }

        void ByteReader_BasicReadRoundTrip() {
            ByteWriter w;
            w.writeU8(0xAB);
            w.writeU16(0xBEEF);
            w.writeU32(0xDEADBEEFu);
            w.writeI32(-123456);
            w.writeF32(3.5f);
            const uint8_t raw[] = { 0x00, 0xFF, 0x10, 0x20 };
            w.writeBytes(raw, sizeof(raw));
            w.writeString("hello");

            ASSERT_EQ(w.hasError(), false);

            ByteReader r;
            r.reset(w.data(), w.size());

            ASSERT_EQ(r.hasError(), false);
            ASSERT_EQ(r.getOffset(), static_cast<size_t>(0));
            ASSERT_EQ(r.remaining(), w.size());

            ASSERT_EQ(r.readU8(), static_cast<uint8_t>(0xAB));
            ASSERT_EQ(r.readU16(), static_cast<uint16_t>(0xBEEF));
            ASSERT_EQ(r.readU32(), static_cast<uint32_t>(0xDEADBEEFu));
            ASSERT_EQ(r.readI32(), static_cast<int32_t>(-123456));

            // For floats compare by bit pattern to avoid issues with -0 / NaN, etc.
            float f = r.readF32();
            const float expected = 3.5f;
            ASSERT_EQ(std::memcmp(&f, &expected, sizeof(float)) == 0, true);

            ASSERT_EQ(r.readU8(), static_cast<uint8_t>(raw[0]));
            ASSERT_EQ(r.readU8(), static_cast<uint8_t>(raw[1]));
            ASSERT_EQ(r.readU8(), static_cast<uint8_t>(raw[2]));
            ASSERT_EQ(r.readU8(), static_cast<uint8_t>(raw[3]));

            ASSERT_EQ(r.readString(), std::string("hello"));
            ASSERT_EQ(r.hasError(), false);
            ASSERT_EQ(r.remaining(), static_cast<size_t>(0));
        }

        void ByteReader_ErrorsAndBounds() {
            // Empty reader: any read should error.
            ByteReader r;
            r.reset(nullptr, 0);
            ASSERT_EQ(r.remaining(), static_cast<size_t>(0));
            ASSERT_EQ(r.readU8(), static_cast<uint8_t>(0));
            ASSERT_EQ(r.hasError(), true);

            // Reading past end should set error and not advance offset.
            const uint8_t buf[] = { 0x11, 0x22, 0x33 };
            r.reset(buf, sizeof(buf));
            ASSERT_EQ(r.readU16(), static_cast<uint16_t>(0x2211));
            ASSERT_EQ(r.getOffset(), static_cast<size_t>(2));
            ASSERT_EQ(r.readU16(), static_cast<uint16_t>(0));
            ASSERT_EQ(r.hasError(), true);
            ASSERT_EQ(r.getOffset(), static_cast<size_t>(2));

            // moveOffset bounds
            r.reset(buf, sizeof(buf));
            ASSERT_EQ(r.hasError(), false);
            r.moveOffset(3);
            ASSERT_EQ(r.getOffset(), static_cast<size_t>(3));
            ASSERT_EQ(r.remaining(), static_cast<size_t>(0));
            r.moveOffset(1); // beyond remaining
            ASSERT_EQ(r.hasError(), true);

            // resetOffset bounds
            r.reset(buf, sizeof(buf));
            ASSERT_EQ(r.hasError(), false);
            r.resetOffset(10);
            ASSERT_EQ(r.hasError(), true);

            // readString bounds: length says 5, but only 3 bytes available.
            ByteWriter w;
            w.writeU32(5);
            w.writeBytes(reinterpret_cast<const uint8_t*>("abc"), 3);
            ASSERT_EQ(w.hasError(), false);

            r.reset(w.data(), w.size());
            ASSERT_EQ(r.readString(), std::string(""));
            ASSERT_EQ(r.hasError(), true);
        }

        void ByteWriter_Errors() {
            ByteWriter w;
            w.writeBytes(nullptr, 1);
            ASSERT_EQ(w.hasError(), true);

            w.clear();
            ASSERT_EQ(w.hasError(), false);
            ASSERT_EQ(w.size(), static_cast<size_t>(0));

            // Writing zero bytes should be a no-op and not error.
            uint8_t dummy = 0xAA;
            w.writeBytes(&dummy, 0);
            ASSERT_EQ(w.hasError(), false);
            ASSERT_EQ(w.size(), static_cast<size_t>(0));
        }
    } // namespace ByteReaderWriter


    namespace NetSessionTests {

        static std::vector<uint8_t> MakePayload(std::initializer_list<uint8_t> bytes) {
            return std::vector<uint8_t>(bytes);
        }

        void QueueOutgoingMessage_BasicAndConsume() {
            NetSession session;

            ASSERT_EQ(session.GetState(), NetSession::State::Open);

            const std::vector<uint8_t> payload = MakePayload({ 0x10, 0x20, 0x30 });
            const bool queued = session.QueueOutgoingMessage(MessageType::Ping, payload);
            ASSERT_EQ(queued, true);
            ASSERT_EQ(session.GetState(), NetSession::State::Open);

            std::vector<uint8_t> bytes = session.ConsumeOutgoingBytes();
            ASSERT_EQ(bytes.empty(), false);

            // After consume buffer should be empty.
            std::vector<uint8_t> bytes2 = session.ConsumeOutgoingBytes();
            ASSERT_EQ(bytes2.empty(), true);

            // Validate produced bytes through ProtocolContract.
            MessageType outType = MessageType::Undefined;
            std::vector<uint8_t> outPayload;
            size_t readBytesCount = 0;

            const ProtocolStatus status = ProtocolContract::TryDeserializeMessage(
                bytes,
                outType,
                outPayload,
                readBytesCount
            );

            ASSERT_EQ(status, ProtocolStatus::Success);
            ASSERT_EQ(outType, MessageType::Ping);
            ASSERT_EQ(outPayload.size(), payload.size());
            ASSERT_EQ(outPayload == payload, true);
            ASSERT_EQ(readBytesCount, bytes.size());
        }

        void PushReceivedBytes_FullMessage_ProducesIncomingMessage() {
            NetSession session;

            const std::vector<uint8_t> payload = MakePayload({ 0xAA, 0xBB, 0xCC });
            std::vector<uint8_t> bytes;

            const ProtocolStatus serializeStatus =
                ProtocolContract::SerializeMessage(bytes, MessageType::Pong, payload);
            ASSERT_EQ(serializeStatus, ProtocolStatus::Success);
            ASSERT_EQ(bytes.empty(), false);

            session.PushReceivedBytes(bytes);

            Message msg;
            const bool hasMessage = session.TryDequeueIncomingMessage(msg);
            ASSERT_EQ(hasMessage, true);
            ASSERT_EQ(msg.type, MessageType::Pong);
            ASSERT_EQ(msg.payload == payload, true);

            Message msg2;
            ASSERT_EQ(session.TryDequeueIncomingMessage(msg2), false);
            ASSERT_EQ(session.GetState(), NetSession::State::Open);
        }

        void PushReceivedBytes_PartialMessage_WaitsUntilEnoughBytes() {
            NetSession session;

            const std::vector<uint8_t> payload = MakePayload({ 0x01, 0x02, 0x03, 0x04, 0x05 });
            std::vector<uint8_t> bytes;

            const ProtocolStatus serializeStatus =
                ProtocolContract::SerializeMessage(bytes, MessageType::Ping, payload);
            ASSERT_EQ(serializeStatus, ProtocolStatus::Success);
            ASSERT_EQ(bytes.size() > 2, true);

            // Feed first part only.
            const size_t split = bytes.size() / 2;
            std::vector<uint8_t> first(bytes.begin(), bytes.begin() + split);
            std::vector<uint8_t> second(bytes.begin() + split, bytes.end());

            session.PushReceivedBytes(first);

            Message msg;
            ASSERT_EQ(session.TryDequeueIncomingMessage(msg), false);
            ASSERT_EQ(session.GetState(), NetSession::State::Open);

            // Feed remaining part.
            session.PushReceivedBytes(second);

            ASSERT_EQ(session.TryDequeueIncomingMessage(msg), true);
            ASSERT_EQ(msg.type, MessageType::Ping);
            ASSERT_EQ(msg.payload == payload, true);
            ASSERT_EQ(session.GetState(), NetSession::State::Open);
        }

        void PushReceivedBytes_TwoMessagesInSingleBuffer_DequeuesBothInOrder() {
            NetSession session;

            std::vector<uint8_t> bytes1;
            std::vector<uint8_t> bytes2;

            const std::vector<uint8_t> payload1 = MakePayload({ 0x11, 0x22 });
            const std::vector<uint8_t> payload2 = MakePayload({ 0x33, 0x44, 0x55 });

            ASSERT_EQ(
                ProtocolContract::SerializeMessage(bytes1, MessageType::Ping, payload1),
                ProtocolStatus::Success
            );
            ASSERT_EQ(
                ProtocolContract::SerializeMessage(bytes2, MessageType::Pong, payload2),
                ProtocolStatus::Success
            );

            std::vector<uint8_t> allBytes;
            allBytes.insert(allBytes.end(), bytes1.begin(), bytes1.end());
            allBytes.insert(allBytes.end(), bytes2.begin(), bytes2.end());

            session.PushReceivedBytes(allBytes);

            Message msg1;
            Message msg2;

            ASSERT_EQ(session.TryDequeueIncomingMessage(msg1), true);
            ASSERT_EQ(session.TryDequeueIncomingMessage(msg2), true);

            ASSERT_EQ(msg1.type, MessageType::Ping);
            ASSERT_EQ(msg1.payload == payload1, true);

            ASSERT_EQ(msg2.type, MessageType::Pong);
            ASSERT_EQ(msg2.payload == payload2, true);

            Message msg3;
            ASSERT_EQ(session.TryDequeueIncomingMessage(msg3), false);
            ASSERT_EQ(session.GetState(), NetSession::State::Open);
        }

        void PushReceivedBytes_PartialValidMessage_DoesNotSetProtocolError() {
            NetSession session;

            std::vector<uint8_t> fullBytes;
            std::vector<uint8_t> payload = { 0xAA, 0xBB, 0xCC };

            const ProtocolStatus serializeStatus =
                ProtocolContract::SerializeMessage(fullBytes, MessageType::Ping, payload);
            ASSERT_EQ(static_cast<int>(serializeStatus), static_cast<int>(ProtocolStatus::Success));

            ASSERT_EQ(fullBytes.size() > 1, true);

            const size_t split = fullBytes.size() / 2;
            std::vector<uint8_t> firstPart(fullBytes.begin(), fullBytes.begin() + split);

            session.PushReceivedBytes(firstPart);

            ASSERT_EQ(static_cast<int>(session.GetState()), static_cast<int>(NetSession::State::Open));

            Message msg;
            ASSERT_EQ(session.TryDequeueIncomingMessage(msg), false);
        }

        void PushReceivedBytes_InvalidBytes_SetsProtocolError() {
            NetSession session;

            std::vector<uint8_t> broken = {
                0xFF, 0xFF,
                0x01, 0x00, 0x00, 0x00,
            };

            session.PushReceivedBytes(broken);

            ASSERT_EQ(static_cast<int>(session.GetState()), static_cast<int>(NetSession::State::ProtocolError));

            Message msg;
            ASSERT_EQ(session.TryDequeueIncomingMessage(msg), false);
        }

        void QueueOutgoingMessage_InvalidType_ReturnsFalse_AndKeepsSessionOpen() {
            NetSession session;

            const bool queued = session.QueueOutgoingMessage(
                MessageType::Undefined,
                MakePayload({ 0x01 })
            );

            ASSERT_EQ(queued, false);
            ASSERT_EQ(session.GetState(), NetSession::State::Open);

            std::vector<uint8_t> bytes = session.ConsumeOutgoingBytes();
            ASSERT_EQ(bytes.empty(), true);
        }

        void QueueOutgoingMessage_TooLargePayload_ReturnsFalse_AndKeepsSessionOpen() {
            NetSession session;

            std::vector<uint8_t> payload(ProtocolContract::MAX_PAYLOAD_SIZE + 1, 0x7F);

            const bool queued = session.QueueOutgoingMessage(MessageType::Ping, payload);
            ASSERT_EQ(queued, false);
            ASSERT_EQ(session.GetState(), NetSession::State::Open);

            std::vector<uint8_t> bytes = session.ConsumeOutgoingBytes();
            ASSERT_EQ(bytes.empty(), true);
        }

        void ProtocolError_BlocksFurtherUsage() {
            NetSession session;

            // Force protocol error through invalid incoming bytes.
            std::vector<uint8_t> broken = {
                0xFF, 0xFF,
                0x01, 0x00, 0x00, 0x00
            };

            session.PushReceivedBytes(broken);

            ASSERT_EQ(session.GetState(), NetSession::State::ProtocolError);

            // Further outgoing enqueue should fail.
            ASSERT_EQ(
                session.QueueOutgoingMessage(MessageType::Ping, MakePayload({ 0x01 })),
                false
            );

            // Pushing valid bytes should do nothing in ProtocolError state.
            std::vector<uint8_t> validBytes;
            ASSERT_EQ(
                ProtocolContract::SerializeMessage(
                    validBytes,
                    MessageType::Ping,
                    MakePayload({ 0xAA })
                ),
                ProtocolStatus::Success
            );

            session.PushReceivedBytes(validBytes);

            Message msg;
            ASSERT_EQ(session.TryDequeueIncomingMessage(msg), false);

            std::vector<uint8_t> outgoing = session.ConsumeOutgoingBytes();
            ASSERT_EQ(outgoing.empty(), true);
        }

        void ConsumeOutgoingBytes_TwoQueuedMessages_ReturnsConcatenatedBytes() {
            NetSession session;

            ASSERT_EQ(
                session.QueueOutgoingMessage(MessageType::Ping, MakePayload({ 0x01, 0x02 })),
                true
            );
            ASSERT_EQ(
                session.QueueOutgoingMessage(MessageType::Pong, MakePayload({ 0x03 })),
                true
            );

            std::vector<uint8_t> bytes = session.ConsumeOutgoingBytes();
            ASSERT_EQ(bytes.empty(), false);

            MessageType type1 = MessageType::Undefined;
            MessageType type2 = MessageType::Undefined;
            std::vector<uint8_t> payload1;
            std::vector<uint8_t> payload2;
            size_t read1 = 0;
            size_t read2 = 0;

            std::vector<uint8_t> remaining = bytes;

            ASSERT_EQ(
                ProtocolContract::TryDeserializeMessage(remaining, type1, payload1, read1),
                ProtocolStatus::Success
            );
            ASSERT_EQ(type1, MessageType::Ping);
            ASSERT_EQ(payload1 == MakePayload({ 0x01, 0x02 }), true);

            remaining.erase(remaining.begin(), remaining.begin() + read1);

            ASSERT_EQ(
                ProtocolContract::TryDeserializeMessage(remaining, type2, payload2, read2),
                ProtocolStatus::Success
            );
            ASSERT_EQ(type2, MessageType::Pong);
            ASSERT_EQ(payload2 == MakePayload({ 0x03 }), true);

            remaining.erase(remaining.begin(), remaining.begin() + read2);
            ASSERT_EQ(remaining.empty(), true);
        }

    } // namespace NetSessionTests

    namespace ProtocolMessages {

        void Ping_RoundTrip() {
            Ping in;
            in.sequence = 123456u;

            ByteWriter w;
            Serialize(w, in);

            ASSERT_EQ(w.hasError(), false);
            ASSERT_EQ(w.size() > 0, true);

            ByteReader r;
            r.reset(w.data(), w.size());

            Ping out;
            const bool ok = Deserialize(r, out);

            ASSERT_EQ(ok, true);
            ASSERT_EQ(r.hasError(), false);
            ASSERT_EQ(out.sequence, in.sequence);
            ASSERT_EQ(r.remaining(), static_cast<size_t>(0));
        }

        void Pong_RoundTrip() {
            Pong in;
            in.sequence = 777u;

            ByteWriter w;
            Serialize(w, in);

            ASSERT_EQ(w.hasError(), false);
            ASSERT_EQ(w.size() > 0, true);

            ByteReader r;
            r.reset(w.data(), w.size());

            Pong out;
            const bool ok = Deserialize(r, out);

            ASSERT_EQ(ok, true);
            ASSERT_EQ(r.hasError(), false);
            ASSERT_EQ(out.sequence, in.sequence);
            ASSERT_EQ(r.remaining(), static_cast<size_t>(0));
        }

        void JoinRequest_RoundTrip() {
            JoinRequest in;
            in.protocolVersion = 1;
            in.playerName = "lexar";

            ByteWriter w;
            Serialize(w, in);

            ASSERT_EQ(w.hasError(), false);
            ASSERT_EQ(w.size() > 0, true);

            ByteReader r;
            r.reset(w.data(), w.size());

            JoinRequest out;
            const bool ok = Deserialize(r, out);

            ASSERT_EQ(ok, true);
            ASSERT_EQ(r.hasError(), false);
            ASSERT_EQ(out.protocolVersion, in.protocolVersion);
            ASSERT_EQ(out.playerName, in.playerName);
            ASSERT_EQ(r.remaining(), static_cast<size_t>(0));
        }

        void JoinRequest_EmptyName_RoundTrip() {
            JoinRequest in;
            in.protocolVersion = 1;
            in.playerName = "";

            ByteWriter w;
            Serialize(w, in);

            ASSERT_EQ(w.hasError(), false);

            ByteReader r;
            r.reset(w.data(), w.size());

            JoinRequest out;
            const bool ok = Deserialize(r, out);

            ASSERT_EQ(ok, true);
            ASSERT_EQ(r.hasError(), false);
            ASSERT_EQ(out.protocolVersion, in.protocolVersion);
            ASSERT_EQ(out.playerName, in.playerName);
            ASSERT_EQ(r.remaining(), static_cast<size_t>(0));
        }

        void JoinAccept_RoundTrip() {
            JoinAccept in;
            in.clientId = 42u;

            ByteWriter w;
            Serialize(w, in);

            ASSERT_EQ(w.hasError(), false);
            ASSERT_EQ(w.size() > 0, true);

            ByteReader r;
            r.reset(w.data(), w.size());

            JoinAccept out;
            const bool ok = Deserialize(r, out);

            ASSERT_EQ(ok, true);
            ASSERT_EQ(r.hasError(), false);
            ASSERT_EQ(out.clientId, in.clientId);
            ASSERT_EQ(r.remaining(), static_cast<size_t>(0));
        }

        void JoinReject_RoundTrip() {
            JoinReject in;
            in.reasonCode = 2u;

            ByteWriter w;
            Serialize(w, in);

            ASSERT_EQ(w.hasError(), false);
            ASSERT_EQ(w.size() > 0, true);

            ByteReader r;
            r.reset(w.data(), w.size());

            JoinReject out;
            const bool ok = Deserialize(r, out);

            ASSERT_EQ(ok, true);
            ASSERT_EQ(r.hasError(), false);
            ASSERT_EQ(out.reasonCode, in.reasonCode);
            ASSERT_EQ(r.remaining(), static_cast<size_t>(0));
        }

        void Ping_DeserializeFailsOnTruncatedData() {
            Ping in;
            in.sequence = 999u;

            ByteWriter w;
            Serialize(w, in);
            ASSERT_EQ(w.hasError(), false);
            ASSERT_EQ(w.size() > 0, true);

            ByteReader r;
            r.reset(w.data(), w.size() - 1);

            Ping out;
            const bool ok = Deserialize(r, out);

            ASSERT_EQ(ok, false);
            ASSERT_EQ(r.hasError(), true);
        }

        void JoinRequest_DeserializeFailsOnTruncatedName() {
            ByteWriter w;
            w.writeU16(1);   // protocolVersion
            w.writeU32(5);   // string length
            w.writeBytes(reinterpret_cast<const uint8_t*>("abc"), 3); // truncated string bytes

            ASSERT_EQ(w.hasError(), false);

            ByteReader r;
            r.reset(w.data(), w.size());

            JoinRequest out;
            const bool ok = Deserialize(r, out);

            ASSERT_EQ(ok, false);
            ASSERT_EQ(r.hasError(), true);
        }

    } // namespace ProtocolMessages

    namespace MessageCodecTests {

        struct FailingEncodeMessage {
        };

        inline void Serialize(ByteWriter& w, const FailingEncodeMessage&)
        {
            w.writeBytes(nullptr, 1);
        }

        struct FailingDecodeMessage {
        };

        inline bool Deserialize(ByteReader&, FailingDecodeMessage&)
        {
            return false;
        }

        void EncodePayload_Ping_Success() {
            Ping in;
            in.sequence = 0x12345678u;

            const std::vector<uint8_t> payload = EncodePayload(in);

            ASSERT_EQ(payload.size(), static_cast<size_t>(4));
            ASSERT_EQ(payload[0], static_cast<uint8_t>(0x78));
            ASSERT_EQ(payload[1], static_cast<uint8_t>(0x56));
            ASSERT_EQ(payload[2], static_cast<uint8_t>(0x34));
            ASSERT_EQ(payload[3], static_cast<uint8_t>(0x12));
        }

        void EncodePayload_JoinRequest_Success() {
            JoinRequest in;
            in.protocolVersion = 7;
            in.playerName = "lex";

            const std::vector<uint8_t> payload = EncodePayload(in);

            ASSERT_EQ(payload.empty(), false);

            ByteReader r;
            r.reset(payload.data(), payload.size());

            JoinRequest out;
            const bool ok = Deserialize(r, out);

            ASSERT_EQ(ok, true);
            ASSERT_EQ(r.hasError(), false);
            ASSERT_EQ(r.remaining(), static_cast<size_t>(0));
            ASSERT_EQ(out.protocolVersion, in.protocolVersion);
            ASSERT_EQ(out.playerName, in.playerName);
        }

        void EncodePayload_WhenSerializeSetsError_ReturnsEmptyVector() {
            FailingEncodeMessage in;

            const std::vector<uint8_t> payload = EncodePayload(in);

            ASSERT_EQ(payload.empty(), true);
        }

        void DecodePayload_Ping_Success() {
            Ping in;
            in.sequence = 42u;

            Message rawMessage;
            rawMessage.type = MessageType::Ping;
            rawMessage.payload = EncodePayload(in);

            Ping out;
            const bool ok = DecodePayload(rawMessage, MessageType::Ping, out);

            ASSERT_EQ(ok, true);
            ASSERT_EQ(out.sequence, in.sequence);
        }

        void DecodePayload_WrongMessageType_ReturnsFalse() {
            Ping in;
            in.sequence = 111u;

            Message rawMessage;
            rawMessage.type = MessageType::Pong;
            rawMessage.payload = EncodePayload(in);

            Ping out;
            out.sequence = 0u;

            const bool ok = DecodePayload(rawMessage, MessageType::Ping, out);

            ASSERT_EQ(ok, false);
            ASSERT_EQ(out.sequence, static_cast<uint32_t>(0));
        }

        void DecodePayload_TruncatedPayload_ReturnsFalse() {
            Message rawMessage;
            rawMessage.type = MessageType::Ping;
            rawMessage.payload = { 0x01, 0x02, 0x03 };

            Ping out;
            const bool ok = DecodePayload(rawMessage, MessageType::Ping, out);

            ASSERT_EQ(ok, false);
        }

        void DecodePayload_TrailingBytes_ReturnsFalse() {
            Ping in;
            in.sequence = 77u;

            Message rawMessage;
            rawMessage.type = MessageType::Ping;
            rawMessage.payload = EncodePayload(in);
            rawMessage.payload.push_back(0xFF);

            Ping out;
            const bool ok = DecodePayload(rawMessage, MessageType::Ping, out);

            ASSERT_EQ(ok, false);
        }

        void DecodePayload_WhenDeserializeReturnsFalse_ReturnsFalse() {
            Message rawMessage;
            rawMessage.type = MessageType::Ping;
            rawMessage.payload = { 0xAA };

            FailingDecodeMessage out;
            const bool ok = DecodePayload(rawMessage, MessageType::Ping, out);

            ASSERT_EQ(ok, false);
        }

    } // namespace MessageCodecTests


    namespace HandshakeTests {
        void Handshake_JoinRequest_Success_ReturnsJoinAccept() {
            NetSession client;
            NetSession server;

            JoinRequest joinRequest;
            joinRequest.protocolVersion = ProtocolContract::PROTOCOL_VERSION;
            joinRequest.playerName = "lex";

            const std::vector<uint8_t> requestPayload = EncodePayload(joinRequest);
            ASSERT_EQ(requestPayload.empty(), false);

            const bool queuedOnClient = client.QueueOutgoingMessage(MessageType::JoinRequest, requestPayload);
            ASSERT_EQ(queuedOnClient, true);

            const std::vector<uint8_t> clientBytes = client.ConsumeOutgoingBytes();
            ASSERT_EQ(clientBytes.empty(), false);

            server.PushReceivedBytes(clientBytes);

            Message serverRawMessage;
            const bool serverHasMessage = server.TryDequeueIncomingMessage(serverRawMessage);
            ASSERT_EQ(serverHasMessage, true);
            ASSERT_EQ(serverRawMessage.type, MessageType::JoinRequest);

            JoinRequest decodedJoinRequest;
            const bool serverDecodedRequest = DecodePayload(serverRawMessage, MessageType::JoinRequest, decodedJoinRequest);
            ASSERT_EQ(serverDecodedRequest, true);
            ASSERT_EQ(decodedJoinRequest.protocolVersion, ProtocolContract::PROTOCOL_VERSION);
            ASSERT_EQ(decodedJoinRequest.playerName, "lex");

            const bool shouldAccept =
                decodedJoinRequest.protocolVersion == ProtocolContract::PROTOCOL_VERSION &&
                !decodedJoinRequest.playerName.empty();

            ASSERT_EQ(shouldAccept, true);

            JoinAccept joinAccept;
            joinAccept.clientId = 42u;

            const std::vector<uint8_t> responsePayload = EncodePayload(joinAccept);
            ASSERT_EQ(responsePayload.empty(), false);

            const bool queuedOnServer = server.QueueOutgoingMessage(MessageType::JoinAccept, responsePayload);
            ASSERT_EQ(queuedOnServer, true);

            const std::vector<uint8_t> serverBytes = server.ConsumeOutgoingBytes();
            ASSERT_EQ(serverBytes.empty(), false);

            client.PushReceivedBytes(serverBytes);

            Message clientRawMessage;
            const bool clientHasMessage = client.TryDequeueIncomingMessage(clientRawMessage);
            ASSERT_EQ(clientHasMessage, true);
            ASSERT_EQ(clientRawMessage.type, MessageType::JoinAccept);

            JoinAccept decodedJoinAccept;
            const bool clientDecodedAccept = DecodePayload(clientRawMessage, MessageType::JoinAccept, decodedJoinAccept);
            ASSERT_EQ(clientDecodedAccept, true);
            ASSERT_EQ(decodedJoinAccept.clientId, static_cast<uint32_t>(42));
        }

        void Handshake_BadProtocol_ReturnsJoinReject() {
            NetSession client;
            NetSession server;

            JoinRequest joinRequest;
			constexpr uint8_t INVALID_PROTOCOL_VERSION = ProtocolContract::PROTOCOL_VERSION + 1;
            joinRequest.protocolVersion = INVALID_PROTOCOL_VERSION;
            joinRequest.playerName = "lex";

            const std::vector<uint8_t> requestPayload = EncodePayload(joinRequest);
            ASSERT_EQ(requestPayload.empty(), false);

            const bool queuedOnClient = client.QueueOutgoingMessage(MessageType::JoinRequest, requestPayload);
            ASSERT_EQ(queuedOnClient, true);

            const std::vector<uint8_t> clientBytes = client.ConsumeOutgoingBytes();
            ASSERT_EQ(clientBytes.empty(), false);

            server.PushReceivedBytes(clientBytes);

            Message serverRawMessage;
            const bool serverHasMessage = server.TryDequeueIncomingMessage(serverRawMessage);
            ASSERT_EQ(serverHasMessage, true);
            ASSERT_EQ(serverRawMessage.type, MessageType::JoinRequest);

            JoinRequest decodedJoinRequest;
            const bool serverDecodedRequest = DecodePayload(serverRawMessage, MessageType::JoinRequest, decodedJoinRequest);
            ASSERT_EQ(serverDecodedRequest, true);
            ASSERT_EQ(decodedJoinRequest.protocolVersion, INVALID_PROTOCOL_VERSION);
            ASSERT_EQ(decodedJoinRequest.playerName, "lex");

            const bool shouldReject =
                decodedJoinRequest.protocolVersion != ProtocolContract::PROTOCOL_VERSION &&
                !decodedJoinRequest.playerName.empty();

            ASSERT_EQ(shouldReject, true);

            JoinReject joinReject;
			joinReject.reasonCode = static_cast<uint8_t>(JoinRejectReason::BAD_PROTOCOL_VERSION);

            const std::vector<uint8_t> responsePayload = EncodePayload(joinReject);
            ASSERT_EQ(responsePayload.empty(), false);

            const bool queuedOnServer = server.QueueOutgoingMessage(MessageType::JoinReject, responsePayload);
            ASSERT_EQ(queuedOnServer, true);

            const std::vector<uint8_t> serverBytes = server.ConsumeOutgoingBytes();
            ASSERT_EQ(serverBytes.empty(), false);

            client.PushReceivedBytes(serverBytes);

            Message clientRawMessage;
            const bool clientHasMessage = client.TryDequeueIncomingMessage(clientRawMessage);
            ASSERT_EQ(clientHasMessage, true);
            ASSERT_EQ(clientRawMessage.type, MessageType::JoinReject);

            JoinReject decodedJoinReject;
            const bool clientDecodedReject = DecodePayload(clientRawMessage, MessageType::JoinReject, decodedJoinReject);
            ASSERT_EQ(clientDecodedReject, true);
            ASSERT_EQ(decodedJoinReject.reasonCode, static_cast<uint8_t>(JoinRejectReason::BAD_PROTOCOL_VERSION));
        }

        void Handshake_EmptyPlayerName_ReturnsJoinReject() {
            NetSession client;
            NetSession server;

            JoinRequest joinRequest;
            joinRequest.protocolVersion = ProtocolContract::PROTOCOL_VERSION;
            joinRequest.playerName = "";

            const std::vector<uint8_t> requestPayload = EncodePayload(joinRequest);
            ASSERT_EQ(requestPayload.empty(), false);

            const bool queuedOnClient = client.QueueOutgoingMessage(MessageType::JoinRequest, requestPayload);
            ASSERT_EQ(queuedOnClient, true);

            const std::vector<uint8_t> clientBytes = client.ConsumeOutgoingBytes();
            ASSERT_EQ(clientBytes.empty(), false);

            server.PushReceivedBytes(clientBytes);

            Message serverRawMessage;
            const bool serverHasMessage = server.TryDequeueIncomingMessage(serverRawMessage);
            ASSERT_EQ(serverHasMessage, true);
            ASSERT_EQ(serverRawMessage.type, MessageType::JoinRequest);

            JoinRequest decodedJoinRequest;
            const bool serverDecodedRequest = DecodePayload(serverRawMessage, MessageType::JoinRequest, decodedJoinRequest);
            ASSERT_EQ(serverDecodedRequest, true);
            ASSERT_EQ(decodedJoinRequest.protocolVersion, ProtocolContract::PROTOCOL_VERSION);
            ASSERT_EQ(decodedJoinRequest.playerName.empty(), true);

            const bool shouldReject =
                decodedJoinRequest.protocolVersion == ProtocolContract::PROTOCOL_VERSION &&
                decodedJoinRequest.playerName.empty();

            ASSERT_EQ(shouldReject, true);

            JoinReject joinReject;
			joinReject.reasonCode = static_cast<uint8_t>(JoinRejectReason::EMPTY_PLAYER_NAME);

            const std::vector<uint8_t> responsePayload = EncodePayload(joinReject);
            ASSERT_EQ(responsePayload.empty(), false);

            const bool queuedOnServer = server.QueueOutgoingMessage(MessageType::JoinReject, responsePayload);
            ASSERT_EQ(queuedOnServer, true);

            const std::vector<uint8_t> serverBytes = server.ConsumeOutgoingBytes();
            ASSERT_EQ(serverBytes.empty(), false);

            client.PushReceivedBytes(serverBytes);

            Message clientRawMessage;
            const bool clientHasMessage = client.TryDequeueIncomingMessage(clientRawMessage);
            ASSERT_EQ(clientHasMessage, true);
            ASSERT_EQ(clientRawMessage.type, MessageType::JoinReject);

            JoinReject decodedJoinReject;
            const bool clientDecodedReject = DecodePayload(clientRawMessage, MessageType::JoinReject, decodedJoinReject);
            ASSERT_EQ(clientDecodedReject, true);
            ASSERT_EQ(decodedJoinReject.reasonCode, static_cast<uint8_t>(JoinRejectReason::EMPTY_PLAYER_NAME));
        }

    } // namespace HandshakeTest

    void RunAllTests() {

        //--------ByteReaderWriter--------//

        std::cout << "ByteReaderWriter START\n";
        RUN_TEST(ByteReaderWriter::ByteWriter_BasicWriteAndRewrite);
        RUN_TEST(ByteReaderWriter::ByteReader_BasicReadRoundTrip);
        RUN_TEST(ByteReaderWriter::ByteReader_ErrorsAndBounds);
        RUN_TEST(ByteReaderWriter::ByteWriter_Errors);
        std::cout << "ByteReaderWriter END\n\n";

        //--------NetSession--------//

        std::cout << "NetSession START\n";
        RUN_TEST(NetSessionTests::QueueOutgoingMessage_BasicAndConsume);
        RUN_TEST(NetSessionTests::PushReceivedBytes_FullMessage_ProducesIncomingMessage);
        RUN_TEST(NetSessionTests::PushReceivedBytes_PartialMessage_WaitsUntilEnoughBytes);
        RUN_TEST(NetSessionTests::PushReceivedBytes_TwoMessagesInSingleBuffer_DequeuesBothInOrder);
        RUN_TEST(NetSessionTests::PushReceivedBytes_PartialValidMessage_DoesNotSetProtocolError);
        RUN_TEST(NetSessionTests::PushReceivedBytes_InvalidBytes_SetsProtocolError);
        RUN_TEST(NetSessionTests::QueueOutgoingMessage_InvalidType_ReturnsFalse_AndKeepsSessionOpen);
        RUN_TEST(NetSessionTests::QueueOutgoingMessage_TooLargePayload_ReturnsFalse_AndKeepsSessionOpen);
        RUN_TEST(NetSessionTests::ProtocolError_BlocksFurtherUsage);
        RUN_TEST(NetSessionTests::ConsumeOutgoingBytes_TwoQueuedMessages_ReturnsConcatenatedBytes);
        std::cout << "NetSession END\n\n";

        //--------ProtocolMessages--------//

        std::cout << "ProtocolMessages START\n";
        RUN_TEST(ProtocolMessages::Ping_RoundTrip);
        RUN_TEST(ProtocolMessages::Pong_RoundTrip);
        RUN_TEST(ProtocolMessages::JoinRequest_RoundTrip);
        RUN_TEST(ProtocolMessages::JoinRequest_EmptyName_RoundTrip);
        RUN_TEST(ProtocolMessages::JoinAccept_RoundTrip);
        RUN_TEST(ProtocolMessages::JoinReject_RoundTrip);
        RUN_TEST(ProtocolMessages::Ping_DeserializeFailsOnTruncatedData);
        RUN_TEST(ProtocolMessages::JoinRequest_DeserializeFailsOnTruncatedName);
        std::cout << "ProtocolMessages END\n\n";

        //--------MessageCodec--------//

        std::cout << "MessageCodec START\n";
        RUN_TEST(MessageCodecTests::EncodePayload_Ping_Success);
        RUN_TEST(MessageCodecTests::EncodePayload_JoinRequest_Success);
        RUN_TEST(MessageCodecTests::EncodePayload_WhenSerializeSetsError_ReturnsEmptyVector);
        RUN_TEST(MessageCodecTests::DecodePayload_Ping_Success);
        RUN_TEST(MessageCodecTests::DecodePayload_WrongMessageType_ReturnsFalse);
        RUN_TEST(MessageCodecTests::DecodePayload_TruncatedPayload_ReturnsFalse);
        RUN_TEST(MessageCodecTests::DecodePayload_TrailingBytes_ReturnsFalse);
        RUN_TEST(MessageCodecTests::DecodePayload_WhenDeserializeReturnsFalse_ReturnsFalse);
        std::cout << "MessageCodec END\n\n";

        //--------HandshakeTest--------//

        std::cout << "HandshakeTest START\n";
        RUN_TEST(HandshakeTests::Handshake_JoinRequest_Success_ReturnsJoinAccept);
        RUN_TEST(HandshakeTests::Handshake_BadProtocol_ReturnsJoinReject);
        RUN_TEST(HandshakeTests::Handshake_EmptyPlayerName_ReturnsJoinReject);
        std::cout << "HandshakeTest END\n\n";

        std::cout << "\nALL TESTS PASSED\n";
    }

} // namespace Tests