#include "bit_stream.h"
#include <vector>
#include <map>

struct Node {
    Node* left = nullptr;   // 0
    Node* right = nullptr;  // 1
    bool is_term = false;
    uint64_t chr = 0;

    ~Node() {
        delete left;
        delete right;
    }
};

class Archiver {
public:
    const uint16_t FILENAME_END = 256;
    const uint16_t ONE_MORE_FILE = 257;
    const uint16_t ARCHIVE_END = 258;

    void Compress(const std::string& archive_name, const std::vector<std::string>& filenames);
    void Decompress(const std::string& archive_name);

    bool Error() const;

protected:
    std::vector<std::pair<uint64_t, uint16_t>> sorted_codes_;
    bool error_ = false;

    std::map<uint16_t, uint64_t> CountChars(const std::string& filename);
    std::map<uint16_t, uint16_t> HuffmanCodeLengths(const std::map<uint16_t, uint64_t>& cnt_chars);
    std::map<uint16_t, std::vector<uint8_t>> HuffmanCanonicCodes(const std::map<uint16_t, uint16_t>& code_lengths);
    void EncodeFile(OBitStream& archive, std::map<uint16_t, std::vector<uint8_t>>& codes, const std::string& filename,
                    bool last);
    bool DecodeFile(IBitStream& archive);
    bool ReadChar(IBitStream& archive, std::map<uint16_t, std::vector<uint8_t>>& codes, uint64_t& c,
                  const Node* const tree);
    Node* GenTree(std::map<uint16_t, std::vector<uint8_t>>& codes);
};
