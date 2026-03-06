#pragma once

#include <cstdint>
#include <vector>
#include <string_view>
#include <bit>
#include <string>

class ByteReader {
public:
	bool hasError() const {
		return error;
	}

	size_t remaining() const {
		if(offset > size) {
			return 0;
		}

		return size - offset;
	}

	std::vector<uint8_t> readBytes(size_t bytesCount) {
		std::vector<uint8_t> bytes;
		bytes.reserve(bytesCount);
		
		for (int i = 0; i < bytesCount; i++) {
			bytes.push_back(readU8());
		}

		return bytes;
	}

	uint8_t readU8() {
		if (remaining() < 1 || data == nullptr) {
			error = true;
			return 0;
		}

		return data[offset++];
	}

	uint16_t readU16() {
		if (remaining() < 2 || data == nullptr) {
			error = true;
			return 0;
		}

		uint16_t val = static_cast<uint16_t>(data[offset]) 
			| (static_cast<uint16_t>(data[offset + 1]) << 8);
		offset += 2;
		return val;
	}

	uint32_t readU32() {
		if (remaining() < 4 || data == nullptr) {
			error = true;
			return 0;
		}

		uint32_t uval = static_cast<uint32_t>(data[offset])
			| (static_cast<uint32_t>(data[offset + 1]) << 8)
			| (static_cast<uint32_t>(data[offset + 2]) << 16)
			| (static_cast<uint32_t>(data[offset + 3]) << 24);
		offset += 4;
		return uval;
	}

	int32_t readI32() {
		uint32_t uval = readU32();
		if (error) {
			return 0;
		}

		return std::bit_cast<int32_t>(uval);
	}

	float readF32() {
		uint32_t uval = readU32();
		if (error) {
			return 0.0f;
		}

		return std::bit_cast<float>(uval);
	}

	std::string readString() {
		size_t strSize = static_cast<size_t>(readU32());
		if(error || remaining() < strSize) {
			error = true;
			return {};
		}

		std::string str(reinterpret_cast<const char*>(data + offset), strSize);
		offset += strSize;
		return str;
	}

	void reset(const uint8_t* data, size_t size) {
		this->data = data;
		this->size = size;
		offset = 0;
		error = false;
	}

	size_t getOffset() const {
		return offset;
	}

	void moveOffset(size_t delta) {
		if (delta > remaining()) {
			error = true;
			return;
		}

		offset += delta;
	}

	void resetOffset(size_t newOffset) {
		if (newOffset > size) {
			error = true;
			return;
		}

		offset = newOffset;
	}

private:
	const uint8_t* data = nullptr;
	size_t size = 0;
	size_t offset = 0;
	bool error = false;
};