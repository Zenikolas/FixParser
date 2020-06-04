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
    std::map<double, uint64_t> _buy; // todo change to DECIMAL
    std::map<double, uint64_t> _sell;
};
}
