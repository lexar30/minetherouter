#pragma once

#include <vector>
#include <queue>
#include <span>

#include "Message.h"
#include "MessageType.h"

namespace mtr::network {
	class NetSession
	{
	public:
		enum class State
		{
			Open = 0,
			ProtocolError,
			Closed,
		};

	public:

		State GetState() const { return _state; }

		bool QueueOutgoingMessage(MessageType type, const std::vector<uint8_t>& payload);

		bool TryDequeueIncomingMessage(Message& outMessage);

		void PushReceivedBytes(std::span<const uint8_t> bytes);
		std::vector<uint8_t> ConsumeOutgoingBytes();

		bool IsOpen() const { return _state == State::Open; }
		void Close();

	private:
		void TryExtractIncomingMessages();

	private:
		std::vector<uint8_t> _incomingBytes;
		std::vector<uint8_t> _outgoingBytes;

		std::queue<Message> _incomingMessages;

		State _state = State::Open;
	};
}