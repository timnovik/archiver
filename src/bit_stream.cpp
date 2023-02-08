#include "bit_stream.h"

uint64_t ReverseBytes(uint64_t buf) {
    uint64_t res = 0;
    for (int i = 0; i < 4; ++i) {
        uint64_t byte1 = (buf >> ((7 - i) * 8)) & 0b11111111ll;
        uint64_t byte2 = (buf & (0b11111111ll << (i * 8))) >> (i * 8);
        res |= (byte1 << (i * 8)) | (byte2 << (7 - i) * 8);
    }
    return res;
}

IBitStream::IBitStream(const std::string& filename) {
    stream_ = StreamType(filename, StreamType::binary | StreamType::in);
    buf_shift_ = 0;
    buffer_ = 0;
}

bool IBitStream::ReadBit(uint64_t& read_to) {
    if (buf_shift_ == 0) {
        if (stream_.eof()) {
            return false;
        }
        buffer_ = 0;
        stream_.read(reinterpret_cast<char*>(&buffer_), sizeof buffer_);
        buffer_ = ReverseBytes(buffer_);
        buf_shift_ = ReadBitsCount();
        if (buf_shift_ == 0) {
            return false;
        }
        buffer_ >>= 64 - buf_shift_;
    }
    read_to &= ~1;
    read_to |= (buffer_ >> --buf_shift_) & 1;
    return true;
}

bool IBitStream::Read(uint64_t& read_to, uint16_t n, uint8_t endianess) {
    if (endianess == L_ENDIAN) {
        for (uint16_t i = 0; i < n; ++i) {
            uint64_t bit = 0;
            if (!ReadBit(bit)) {
                return false;
            }
            read_to ^= read_to & (1 << i);
            read_to |= (bit & 1) << i;
        }
    } else {
        for (uint16_t i = 0; i < n; ++i) {
            read_to <<= 1;
            if (!ReadBit(read_to)) {
                return false;
            }
        }
    }
    return true;
}

uint8_t IBitStream::ReadBitsCount() {
    return stream_.gcount() * 8;
}

OBitStream::OBitStream(const std::string& filename) {
    stream_ = StreamType(filename, StreamType::binary | StreamType::out);
    buf_shift_ = 64;
    buffer_ = 0;
}

void OBitStream::WriteBit(uint64_t write) {
    if (buf_shift_ == 0) {
        Flush();
    }
    buffer_ |= (write & 1) << --buf_shift_;
}

void OBitStream::Write(uint64_t write, uint16_t n, uint8_t endianess) {
    if (endianess == L_ENDIAN) {
        for (uint16_t i = 0; i < n; ++i) {
            WriteBit(write >> (n - i - 1));
        }
    } else {
        for (uint16_t i = 0; i < n; ++i) {
            WriteBit(write >> i);
        }
    }
}

void OBitStream::Flush() {
    uint8_t unused_bytes = buf_shift_ / 8;
    buffer_ = ReverseBytes(buffer_);
    stream_.write(reinterpret_cast<char*>(&buffer_), 8 - unused_bytes);
    buf_shift_ = 64;
    buffer_ = 0;
}

OBitStream::~OBitStream() {
    Flush();
}

bool OBitStream::Good() {
    return stream_.good();
}
