#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <DataReaderHandler.h>

namespace fix_parser {
class DataReader {
public:
    explicit DataReader(DataReaderHandler *handler) : _handler(handler) { };

    void load(const std::string &filename);

private:
    DataReaderHandler *_handler;
};
}
