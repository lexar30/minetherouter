#pragma once

#include <cstdint>
#include <vector>
#include <string_view>
#include <bit>

namespace mtr::network::core {
	class ByteWriter {
	public:
		ByteWriter() {
			constexpr size_t initialSize = 64;
			buffer.reserve(initialSize);
			error = false;
		}

		bool hasError() const {
			return error;
		}

		void writeU8(uint8_t b) {
			buffer.push_back(b);
		}

		void writeU16(uint16_t b) {
			writeU8(b & 0xFF);
			writeU8((b >> 8) & 0xFF);
		}

		void rewriteU16(size_t offset, uint16_t b) {
			constexpr size_t requiredSize = 2;
			if (buffer.size() < requiredSize || offset > buffer.size() - requiredSize) {
				error = true;
				return;
			}

			buffer[offset] = b & 0xFF;
			buffer[offset + 1] = (b >> 8) & 0xFF;
		}

		void writeU32(uint32_t b) {
			writeU8(b & 0xFF);
			writeU8((b >> 8) & 0xFF);
			writeU8((b >> 16) & 0xFF);
			writeU8((b >> 24) & 0xFF);
		}

		void rewriteU32(size_t offset, uint32_t b) {
			constexpr size_t requiredSize = 4;
			if (buffer.size() < requiredSize || offset > buffer.size() - requiredSize) {
				error = true;
				return;
			}

			buffer[offset] = b & 0xFF;
			buffer[offset + 1] = (b >> 8) & 0xFF;
			buffer[offset + 2] = (b >> 16) & 0xFF;
			buffer[offset + 3] = (b >> 24) & 0xFF;
		}

		void writeI32(int32_t b) {
			writeU32(static_cast<uint32_t>(b));
		}

		void writeF32(float b) {
			writeU32(std::bit_cast<uint32_t>(b));
		}

		void writeBytes(const uint8_t* data, size_t size) {
			if (size == 0) {
				return;
			}

			if (data == nullptr) {
				error = true;
				return;
			}

			buffer.insert(buffer.end(), data, data + size);
		}

		void writeString(const std::string_view str) {
			// todo: докинуть проверку size, не критично
			writeU32(static_cast<uint32_t>(str.size()));
			writeBytes(reinterpret_cast<const uint8_t*>(str.data()), str.size());
		}

		void clear() {
			buffer.clear();
			error = false;
		}

		const uint8_t* data() const {
			return buffer.data();
		}

		size_t size() const {
			return buffer.size();
		}

	private:
		std::vector<uint8_t> buffer;
		bool error = false;
	};
}