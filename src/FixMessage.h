#pragma once

#include "OrderBook.h"

#include <string_view>

namespace fix_parser {
    class FixMessage {
    public:
        enum class UpdateAction {
            New,
            Change,
            Delete
        };

        FixMessage() = default;

        void setUpdateAction(const std::string_view &value);

        void setSide(const std::string_view &value);

        void setPrice(const std::string_view &value);

        void setQty(const std::string_view &value, uint64_t volumeMultiplier);

        const std::optional <UpdateAction> &getUpdateAction() const { return _update_action; }

        const std::optional <Side> &getSide() const { return _side; }

        const std::optional<double> &getPrice() const { return _price; }

        const std::optional <uint64_t> &getQty() const { return _qty; }

        bool isCompleted() const {
            return _update_action && _side && _price && _qty;
        }

    private:
        std::optional <UpdateAction> _update_action;
        std::optional <Side> _side;
        std::optional<double> _price;
        std::optional <uint64_t> _qty;
    };
}