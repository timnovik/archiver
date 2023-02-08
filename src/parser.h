#include <vector>
#include <string>

enum class Mode { compress, decompress, help };

class Parser {
public:
    Parser();
    Parser(int size, const std::vector<std::string>& args);
    bool Parsed() const;
    Mode GetMode() const;
    const std::string& Archive() const;
    const std::vector<std::string>& Files() const;

protected:
    bool parsed_;
    Mode mode_;
    std::vector<std::string> files_;
    std::string archive_name_;
};
