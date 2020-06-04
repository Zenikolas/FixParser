#include "OrderBook.h"

namespace fix_parser {
void OrderBook::add_change(const LevelData &level) {
    if (level.side == Side::BUY) {
        _buy[level.price] = level.qty;
    } else {
        _sell[level.price] = level.qty;
    }
}

void OrderBook::del(const LevelData &level) {
    if (level.side == Side::BUY) {
        _buy.erase(level.price);
    } else {
        _sell.erase(level.price);
    }
}

std::string OrderBook::to_s(const size_t depth) {
    std::string out_string = "Total SELL: " + std::to_string(_sell.size()) + "\n";

    std::string temp_string;

    if (depth < _sell.size()) {
        out_string += "...\n";
    }

    uint16_t count = 0;
    for (auto &it : _sell) {
        temp_string = "[" + std::to_string(count) + "]: price: " + std::to_string(it.first) +
                      " (" + std::to_string(it.second) + ")\n" + temp_string;
        count++;
        if (count >= depth) {
            break;
        }
    }

    out_string += temp_string;
    out_string += "==================\n";

    count = 0;
    for (auto it = _buy.rbegin(); it != _buy.rend(); it++) {
        out_string += "[" + std::to_string(count) + "]: price: " + std::to_string(it->first) +
                      " (" + std::to_string(it->second) + ")\n";
        count++;
        if (count >= depth) {
            break;
        }
    }

    if (depth < _buy.size()) {
        out_string += "...\n";
    }

    out_string += "Total BUY: " + std::to_string(_buy.size()) + "\n";
    return out_string;
}
}
