#pragma once

#include <OrderBook.h>

#include <string_view>

namespace fix_parser {
class FixMessage {
    // FixMessage class gathers all required ticks fields for updating Order Book
public:
    enum class UpdateAction {
        New,
        Change,
        Delete
    };

    FixMessage() = default;

    bool set_update_action(const std::string_view &value);

    bool set_side(const std::string_view &value);

    bool set_price(const std::string_view &value);

    bool set_qty(const std::string_view &value, uint64_t volumeMultiplier);

    const std::optional <UpdateAction> &get_update_action() const { return _update_action; }

    const std::optional <Side> &get_side() const { return _side; }

    const std::optional<double> &get_price() const { return _price; }

    const std::optional <uint64_t> &get_qty() const { return _qty; }

    bool is_completed() const {
        return _update_action && _side && _price && _qty;
    }
    // returns true if of all required fields are set and false otherwise

private:
    std::optional <UpdateAction> _update_action;
    std::optional <Side> _side;
    std::optional<double> _price;
    std::optional <uint64_t> _qty;
};
}