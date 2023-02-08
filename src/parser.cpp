#include "parser.h"

Parser::Parser() = default;

Parser::Parser(int size, const std::vector<std::string>& args) {
    parsed_ = true;
    if (!size) {
        parsed_ = false;
        return;
    }
    std::string mode(args[0]);
    for (int i = 2; i < size; ++i) {
        files_.push_back(args[i]);
    }

    std::string mode_compress = "-c";
    std::string mode_decompress = "-d";
    std::string mode_help = "-h";
    if (mode == mode_compress) {
        mode_ = Mode::compress;
        if (size < 3) {
            parsed_ = false;
        } else {
            archive_name_ = args[1];
        }
    } else if (mode == mode_decompress) {
        mode_ = Mode::decompress;
        if (size != 2) {
            parsed_ = false;
        } else {
            archive_name_ = args[1];
        }
    } else if (mode == mode_help) {
        mode_ = Mode::help;
        if (size > 1) {
            parsed_ = false;
        }
    } else {
        parsed_ = false;
    }
}

bool Parser::Parsed() const {
    return parsed_;
}

Mode Parser::GetMode() const {
    return mode_;
}

const std::string& Parser::Archive() const {
    return archive_name_;
}

const std::vector<std::string>& Parser::Files() const {
    return files_;
}
