#include <sstream>
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

    std::stringstream sell_output;
    sell_output << RED;

    auto sell_end_it = _sell.rend();
    if (depth < _sell.size()) {
        sell_output << "...\n";
        std::advance(sell_end_it, -depth);
    } else {
        sell_end_it = _sell.rbegin();
    }

    uint16_t count = depth;
    for (auto it = sell_end_it; it != _sell.rend(); ++it) {
        sell_output << "[" + std::to_string(--count) + "]: price: " + std::to_string(it->first) +
                       " (" + std::to_string(it->second) + ")\n";
    }

    sell_output << RESET;
    out_string += sell_output.str();
    out_string += "==================\n";

    std::stringstream buy_output;
    buy_output << GREEN;
    count = 0;
    for (auto it = _buy.rbegin(); it != _buy.rend(); it++) {
        buy_output << "[" + std::to_string(count++) + "]: price: " + std::to_string(it->first) +
                      " (" + std::to_string(it->second) + ")\n";
        if (count >= depth) {
            break;
        }
    }

    if (depth < _buy.size()) {
        buy_output << "...\n";
    }

    buy_output << RESET;
    out_string += buy_output.str();
    out_string += "Total BUY: " + std::to_string(_buy.size()) + "\n";
    return out_string;
}
}
