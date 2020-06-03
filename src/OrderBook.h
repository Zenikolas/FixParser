#pragma once

#include <string>
#include <map>

namespace fix_parser {
enum Side : int8_t {
    SELL = -1, BUY = 1
};

struct LevelData {
    Side side;
    double price;
    uint64_t qty;
};

class OrderBook {
public:
    std::string to_s(const size_t depth = 5);

    void add_change(const LevelData &level);
    void del(const LevelData &level);

private:
    const std::string_view RESET = "\033[0m";
    const std::string_view RED = "\033[31m";
    const std::string_view GREEN = "\033[32m";

    std::map<double, uint64_t> _buy; // todo change to DECIMAL
    std::map<double, uint64_t> _sell;
};
}
