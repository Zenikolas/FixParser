#pragma once

namespace fix_parser {
class DataReaderHandler {
public:
    virtual ~DataReaderHandler() = default;

    virtual void handle_line(const std::string &line) = 0;
};

}
