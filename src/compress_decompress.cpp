#include "compress_decompress.h"
#include <queue>
#include <algorithm>
#include <set>

bool Archiver::Error() const {
    return error_;
}

void Archiver::Compress(const std::string& archive_name, const std::vector<std::string>& filenames) {
    OBitStream archive(archive_name);
    for (size_t i = 0; i < filenames.size() && !error_; ++i) {
        const auto& filename = filenames[i];
        auto cnt_chars = CountChars(filename);
        auto code_lengths = HuffmanCodeLengths(cnt_chars);
        auto codes = HuffmanCanonicCodes(code_lengths);
        EncodeFile(archive, codes, filename, i == filenames.size() - 1);
    }
}

std::map<uint16_t, uint64_t> Archiver::CountChars(const std::string& filename) {
    std::map<uint16_t, uint64_t> result;
    result[FILENAME_END] = 1;
    result[ONE_MORE_FILE] = 1;
    result[ARCHIVE_END] = 1;
    for (size_t i = filename.size(); i > 0 && filename[i - 1] != '/';) {
        ++result[filename[--i]];
    }
    IBitStream stream(filename);
    uint64_t c = 0;
    while (stream.Read(c, 8)) {
        ++result[c];
        c = 0;
    }
    return result;
}

std::map<uint16_t, uint16_t> Archiver::HuffmanCodeLengths(const std::map<uint16_t, uint64_t>& cnt_chars) {
    using Node = std::pair<uint64_t, std::vector<uint16_t>>;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> queue;

    for (auto [c, priority] : cnt_chars) {
        queue.push({priority, {c}});
    }

    std::map<uint16_t, uint16_t> code_lengths;
    while (queue.size() > 1) {
        auto l = queue.top();
        queue.pop();
        auto r = queue.top();
        queue.pop();

        if (l.second.front() > r.second.front()) {
            std::swap(l, r);
        }

        l.first += r.first;
        for (auto c : l.second) {
            ++code_lengths[c];
        }
        for (auto c : r.second) {
            ++code_lengths[c];
            l.second.push_back(c);
        }

        queue.push(l);
    }
    return code_lengths;
}

std::map<uint16_t, std::vector<uint8_t>> Archiver::HuffmanCanonicCodes(
    const std::map<uint16_t, uint16_t>& code_lengths) {
    sorted_codes_.clear();
    for (auto [chr, code_length] : code_lengths) {
        sorted_codes_.emplace_back(code_length, chr);
    }

    std::sort(sorted_codes_.begin(), sorted_codes_.end());

    std::map<uint16_t, std::vector<uint8_t>> codes;
    codes[sorted_codes_[0].second].assign(sorted_codes_[0].first, 0);

    for (size_t i = 1; i < sorted_codes_.size(); ++i) {
        codes[sorted_codes_[i].second] = codes[sorted_codes_[i - 1].second];

        size_t j = codes[sorted_codes_[i].second].size();
        while (j > 0 && codes[sorted_codes_[i].second][j - 1] == 1) {
            codes[sorted_codes_[i].second][--j] = 0;
        }
        codes[sorted_codes_[i].second][--j] = 1;

        codes[sorted_codes_[i].second].reserve(sorted_codes_[i].first);

        for (size_t s = codes[sorted_codes_[i].second].size(); s < sorted_codes_[i].first; ++s) {
            codes[sorted_codes_[i].second].push_back(0);
        }
    }
    return codes;
}

void Archiver::EncodeFile(OBitStream& archive, std::map<uint16_t, std::vector<uint8_t>>& codes,
                          const std::string& filename, bool last) {
    archive.Write(codes.size(), 9, archive.L_ENDIAN);

    std::vector<std::uint16_t> length_count;

    for (auto [length, chr] : sorted_codes_) {
        archive.Write(chr, 9, archive.L_ENDIAN);
        while (length > length_count.size()) {
            length_count.push_back(0);
        }
        ++length_count[length - 1];
    }

    for (auto el : length_count) {
        archive.Write(el, 9, archive.L_ENDIAN);
    }

    size_t i = filename.size();
    for (; i > 0 && filename[i - 1] != '/'; --i) {
    }
    std::string s = filename.substr(i);

    for (uint16_t c : s) {
        for (uint8_t c1 : codes[c]) {
            archive.WriteBit(c1);
        }
    }

    for (uint8_t c : codes[FILENAME_END]) {
        archive.WriteBit(c);
    }

    IBitStream stream(filename);
    uint64_t c = 0;
    while (stream.Read(c, 8)) {
        for (uint8_t c1 : codes[c]) {
            archive.WriteBit(c1);
        }
        c = 0;
    }

    if (last) {
        for (uint8_t c1 : codes[ARCHIVE_END]) {
            archive.WriteBit(c1);
        }
    } else {
        for (uint8_t c1 : codes[ONE_MORE_FILE]) {
            archive.WriteBit(c1);
        }
    }
    error_ = !archive.Good();
}

void Archiver::Decompress(const std::string& archive_name) {
    IBitStream archive(archive_name);
    while (DecodeFile(archive) && !error_) {
    }
}

bool Archiver::DecodeFile(IBitStream& archive) {
    uint64_t symbols_count = 0;
    error_ = !archive.Read(symbols_count, 9);
    if (error_) {
        return false;
    }

    std::vector<uint16_t> symbols;
    uint64_t c = 0;
    while (symbols.size() != symbols_count && archive.Read(c, 9)) {
        symbols.push_back(c);
        c = 0;
    }

    error_ = symbols.size() != symbols_count;
    if (error_) {
        return false;
    }

    std::map<uint16_t, uint16_t> code_lengths;
    size_t i = 0;
    uint16_t cur_length = 1;
    while (i < symbols.size() && archive.Read(c, 9)) {
        while (c > 0) {
            code_lengths[symbols[i++]] = cur_length;
            --c;
        }
        ++cur_length;
    }

    error_ = i != symbols.size();
    if (error_) {
        return false;
    }

    auto codes = HuffmanCanonicCodes(code_lengths);

    auto tree = GenTree(codes);

    std::string filename;

    while (ReadChar(archive, codes, c, tree) && codes[c] != codes[FILENAME_END]) {
        filename += static_cast<char>(c);
        c = 0;
    }

    error_ = codes[c] != codes[FILENAME_END];
    if (error_) {
        delete tree;
        return false;
    }

    OBitStream file(filename);
    while (ReadChar(archive, codes, c, tree) && codes[c] != codes[ARCHIVE_END] && codes[c] != codes[ONE_MORE_FILE]) {
        file.Write(c, 8, file.L_ENDIAN);
        c = 0;
    }

    error_ = codes[c] != codes[ARCHIVE_END] && codes[c] != codes[ONE_MORE_FILE];
    if (error_) {
        delete tree;
        return false;
    }
    delete tree;
    return codes[c] == codes[ONE_MORE_FILE];
}

bool Archiver::ReadChar(IBitStream& archive, std::map<uint16_t, std::vector<uint8_t>>& codes, uint64_t& c,
                        const Node* const tree) {
    auto cur = tree;
    uint64_t bit = 0;
    while (!error_ && !cur->is_term && archive.ReadBit(bit)) {
        if (bit == 0) {
            error_ = cur->left == nullptr;
            cur = cur->left;
        } else {
            error_ = cur->right == nullptr;
            cur = cur->right;
        }
        bit = 0;
    }
    if (error_) {
        return false;
    }
    c = cur->chr;
    return true;
}

Node* Archiver::GenTree(std::map<uint16_t, std::vector<uint8_t>>& codes) {
    Node* result = new Node();
    for (auto& [c, code] : codes) {
        auto cur = result;
        for (size_t i = 0; i < code.size(); ++i) {
            if (code[i] == 0) {
                if (cur->left == nullptr) {
                    cur->left = new Node();
                }
                cur = cur->left;
            } else {
                if (cur->right == nullptr) {
                    cur->right = new Node();
                }
                cur = cur->right;
            }
        }
        cur->is_term = true;
        cur->chr = c;
    }
    return result;
}
