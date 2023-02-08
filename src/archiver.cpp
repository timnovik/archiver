#include "parser.h"
#include "messenger.h"
#include "compress_decompress.h"
#include <iostream>

int main(int argc, char** argv) {
    std::vector<std::string> arg_vector(argc - 1);
    for (int i = 1; i < argc; ++i) {
        arg_vector[i - 1] = std::string(argv[i]);
    }

    Messenger messenger(&std::cout);
    Parser parser(argc - 1, arg_vector);

    if (!parser.Parsed()) {
        messenger.SendMessage(Message::cli_error);
        return 111;
    }

    if (parser.GetMode() == Mode::help) {
        messenger.SendMessage(Message::help);
        return 0;
    }

    if (parser.GetMode() == Mode::compress) {
        Archiver archiver;
        archiver.Compress(parser.Archive(), parser.Files());
        if (archiver.Error()) {
            messenger.SendMessage(Message::compress_error);
            return 111;
        }
        return 0;
    }

    if (parser.GetMode() == Mode::decompress) {
        Archiver archiver;
        archiver.Decompress(parser.Archive());
        if (archiver.Error()) {
            messenger.SendMessage(Message::decompress_error);
            return 111;
        }
        return 0;
    }
    return 0;
}
