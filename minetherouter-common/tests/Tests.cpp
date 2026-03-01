#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

#include "Tests.h"
#include <core/ByteReader.h>
#include <core/ByteWriter.h>

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



    void RunAllTests() {

        //--------ByteReaderWriter--------//

        std::cout << "ByteReaderWriter START\n";
        RUN_TEST(ByteReaderWriter::ByteWriter_BasicWriteAndRewrite);
        RUN_TEST(ByteReaderWriter::ByteReader_BasicReadRoundTrip);
        RUN_TEST(ByteReaderWriter::ByteReader_ErrorsAndBounds);
        RUN_TEST(ByteReaderWriter::ByteWriter_Errors);
        std::cout << "ByteReaderWriter END\n\n";

        std::cout << "\nALL TESTS PASSED\n";
    }

} // namespace Tests