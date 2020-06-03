#include "FixMessage.h"

#include "ParseUtils.h"

#include <iostream>

namespace fix_parser {
    void FixMessage::setUpdateAction(const std::string_view &value) {
        if (value.size() != 1) {
            std::cerr << "Invalid size for MDUpdateAction value: " << value.size() << std::endl;
            return;
        }

        switch (value[0] - '0') {
            case 0:
                _update_action = UpdateAction::New;
                break;
            case 1:
                _update_action = UpdateAction::Change;
                break;
            case 2:
                _update_action = UpdateAction::Delete;
                break;
            default:
                std::cerr << "Unknown MDUpdateAction value: " << value << std::endl;
                break;
        }

        _side.reset();
        _price.reset();
        _qty.reset();
    }

    void FixMessage::setSide(const std::string_view &value) {
        if (value.size() != 1) {
            std::cerr << "Invalid size for MDEntryType value: " << value.size() << std::endl;
            return;
        }

        switch (value[0] - '0') {
            case 0:
                _side = Side::BUY;
                break;
            case 1:
                _side = Side::SELL;
                break;
            case 2:
                std::cerr << "skipping MDEntryType - trade" << std::endl;
                break;
            default:
                std::cerr << "Unknown MDEntryType value: " << value << std::endl;
                break;
        }

        _price.reset();
        _qty.reset();
    }

    void FixMessage::setPrice(const std::string_view &value) {
        try {
            _price = std::stod(value.data());
        } catch (const std::invalid_argument &exception) {
            std::cerr << exception.what() << std::endl;
            return;
        } catch (const std::out_of_range &exception) {
            std::cerr << exception.what() << std::endl;
            return;
        }

        _qty.reset();
    }

    void FixMessage::setQty(const std::string_view &value, uint64_t volumeMultiplier) {
        auto[qty, ec] = parseNumber<uint64_t>(value);
        if (ec != std::errc()) {
            std::cerr << "Failed to parse MDEntrySize tag value: " << value << std::endl;
            return;
        }

        _qty = qty * volumeMultiplier;
    }
}
