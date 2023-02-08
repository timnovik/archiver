#pragma once

#include <string>
#include <fstream>
#include <iostream>

uint64_t ReverseBytes(uint64_t buf);

class IBitStream {
public:
    using StreamType = std::basic_fstream<char, std::char_traits<char>>;

    static const uint8_t L_ENDIAN = 0;
    static const uint8_t B_ENDIAN = 1;

    explicit IBitStream(const std::string& filename);

    bool ReadBit(uint64_t& read_to);

    bool Read(uint64_t& read_to, uint16_t n, uint8_t endianess = B_ENDIAN);

    uint8_t ReadBitsCount();

protected:
    StreamType stream_;
    uint64_t buffer_;
    uint8_t buf_shift_;
};

class OBitStream {
public:
    static const uint8_t L_ENDIAN = 0;
    static const uint8_t B_ENDIAN = 1;

    using StreamType = std::basic_fstream<char, std::char_traits<char>>;

    explicit OBitStream(const std::string& filename);

    void WriteBit(uint64_t write);

    void Write(uint64_t write, uint16_t n, uint8_t endianess = B_ENDIAN);

    void Flush();

    bool Good();

    ~OBitStream();

protected:
    StreamType stream_;
    uint64_t buffer_;
    int8_t buf_shift_;
};
