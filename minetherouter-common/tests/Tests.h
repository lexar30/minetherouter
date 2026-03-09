#pragma once

#include <iostream>
#include <type_traits>
#include <cstdlib>

namespace Tests {
    void RunAllTests();

    void ByteWriter_BasicWriteAndRewrite();
    void ByteReader_BasicReadRoundTrip();
    void ByteReader_ErrorsAndBounds();
    void ByteWriter_Errors();

    void QueueOutgoingMessage_BasicAndConsume();
    void PushReceivedBytes_FullMessage_ProducesIncomingMessage();
    void PushReceivedBytes_PartialMessage_WaitsUntilEnoughBytes();
    void PushReceivedBytes_TwoMessagesInSingleBuffer_DequeuesBothInOrder();
    void PushReceivedBytes_PartialValidMessage_DoesNotSetProtocolError();
    void PushReceivedBytes_InvalidBytes_SetsProtocolError();
    void QueueOutgoingMessage_InvalidType_ReturnsFalse_AndKeepsSessionOpen();
    void QueueOutgoingMessage_TooLargePayload_ReturnsFalse_AndKeepsSessionOpen();
    void ProtocolError_BlocksFurtherUsage();
    void ConsumeOutgoingBytes_TwoQueuedMessages_ReturnsConcatenatedBytes();
    
    void Ping_RoundTrip();
    void Pong_RoundTrip();
    void JoinRequest_RoundTrip();
    void JoinRequest_EmptyName_RoundTrip();
    void JoinAccept_RoundTrip();
    void JoinReject_RoundTrip();
    void Ping_DeserializeFailsOnTruncatedData();
    void JoinRequest_DeserializeFailsOnTruncatedName();
    
    void EncodePayload_Ping_Success();
    void EncodePayload_JoinRequest_Success();
    void EncodePayload_WhenSerializeSetsError_ReturnsEmptyVector();
    void DecodePayload_Ping_Success();
    void DecodePayload_WrongMessageType_ReturnsFalse();
    void DecodePayload_TruncatedPayload_ReturnsFalse();
    void DecodePayload_TrailingBytes_ReturnsFalse();
    void DecodePayload_WhenDeserializeReturnsFalse_ReturnsFalse();
    
    void Handshake_JoinRequest_Success_ReturnsJoinAccept();
    void Handshake_BadProtocol_ReturnsJoinReject();
    void Handshake_EmptyPlayerName_ReturnsJoinReject();

    template<typename T>
    auto ToPrintableValue(const T& value)
    {
        if constexpr (std::is_enum_v<T>) {
            return static_cast<std::underlying_type_t<T>>(value);
        }
        else {
            return value;
        }
    }

    #define ASSERT_EQ(a,b) do { \
        auto _va = (a); \
        auto _vb = (b); \
        if (_va != _vb) { \
            std::cerr << "ASSERT_EQ failed at " << __FILE__ << ":" << __LINE__ << "\n" \
                      << " expected: " << #a << " == " << #b << "\n" \
                      << " actual:   " << ToPrintableValue(_va) << " != " << ToPrintableValue(_vb) << "\n"; \
            std::exit(1); \
        } \
    } while(0)

    #define RUN_TEST(name) do { \
        std::cout << "[ RUN      ] " << #name << "\n"; \
        name(); \
        std::cout << "[     OK   ] " << #name << "\n"; \
        std::cout << "------------" << "\n"; \
    } while(0)
}