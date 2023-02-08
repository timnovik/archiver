#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>

enum class Message { cli_error, compress_error, decompress_error, help };

class Messenger {
public:
    explicit Messenger(std::ostream* stream_ptr);
    void SendMessage(Message message);

protected:
    std::ostream* stream_ptr_;
    std::unordered_map<Message, std::string> messages_ = {
        {Message::cli_error,
         "Error: couldn't match arguments given to one of possible operations.\n"
         "Type \"archiver -h\" to see correct templates to call this program.\n\n"},
        {Message::compress_error,
         "Error: couldn't compress files.\n"
         "Some files that were given don't exist or it's not possible to create archive here.\n\n"},
        {Message::decompress_error,
         "Error: couldn't decompress files.\n"
         "Archive doesn't exist or isn't valid.\n\n"},
        {Message::help,
         "This is an archiver written by tnovikov.\n"
         "Type \"archiver -c archive_name file_name1 [file_name2 ...]\" to compress files to archive_name.\n"
         "Type \"archiver -d archive_name\" to decompress archive_name.\n"
         "Type \"archiver -h\" to see this text.\n\n"}};
};
