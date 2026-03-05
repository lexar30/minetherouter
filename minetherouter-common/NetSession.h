#pragma once

#include <vector>
#include <queue>

enum class NetSessionState
{
	Undefined = 0,
	Connecting,
	Connected,
	ProtocolError,
	Closed,
};

enum class MessageType;

class NetSession
{
	struct Message {
		MessageType type = MessageType::Undefined;
		std::vector<uint8_t> payload;
	};

private:
	static constexpr size_t BUFFER_SIZE = 4096;

public:
	NetSession() : _buffer(BUFFER_SIZE) {}

	void AcceptBytes(uint8_t* incomingBytes, size_t size);

	void ProcessMessages();

	NetSessionState GetState() const { return _state; }

private:
	void ProcessMessage(const Message& msg);
	bool TryPopMessage(Message& outMessage);

	void EnqueueMessage(MessageType type, const std::vector<uint8_t>& payload);

private:
	std::vector<uint8_t> _buffer;
	std::vector<uint8_t> _incoming;
	std::vector<uint8_t> _outcoming;
						 
	std::queue<Message> _unprocessedMessagesIn;
	std::queue<Message> _unprocessedMessagesOut;

	NetSessionState _state = NetSessionState::Undefined;
};

