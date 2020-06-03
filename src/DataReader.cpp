#include "DataReader.h"

namespace fix_parser {
void DataReader::load(const std::string &filename) {
    std::ifstream ifs;
    ifs.open(filename, std::ifstream::in);

    if (!ifs.good()) {
        throw std::runtime_error("Can't load config file: " + filename);
    }

    std::string line{};
    while (std::getline(ifs, line)) {
        _handler->handle_line(line);
    }
}
}
