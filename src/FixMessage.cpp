#include <FixMessage.h>

#include <ParseUtils.h>

#include <iostream>

namespace fix_parser {
    bool FixMessage::set_update_action(const std::string_view &value) {
        if (value.size() != 1) {
            std::cerr << "Invalid size for MDUpdateAction value: " << value.size() << std::endl;
            return false;
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
                std::cerr << "skipping MDUpdateAction value: " << value << std::endl;
                return false;
        }

        return true;
    }

    bool FixMessage::set_side(const std::string_view &value) {
        if (value.size() != 1) {
            std::cerr << "Invalid size for MDEntryType value: " << value.size() << std::endl;
            return false;
        }

        switch (value[0] - '0') {
            case 0:
                _side = Side::BUY;
                break;
            case 1:
                _side = Side::SELL;
                break;
            default:
                std::cerr << "skipping MDEntryType value: " << value << std::endl;
                return false;
        }

        return true;
    }

    bool FixMessage::set_price(const std::string_view &value) {
        try {
            _price = std::stod(value.data());
        } catch (const std::invalid_argument &exception) {
            std::cerr << exception.what() << std::endl;
            return false;
        } catch (const std::out_of_range &exception) {
            std::cerr << exception.what() << std::endl;
            return false;
        }

        return true;
    }

    bool FixMessage::set_qty(const std::string_view &value, uint64_t volume_multiplier) {
        auto[qty, ec] = parse_number<uint64_t>(value);
        if (ec != std::errc()) {
            std::cerr << "Failed to parse MDEntrySize tag value: " << value << std::endl;
            return false;
        }

        _qty = qty * volume_multiplier;
        return true;
    }
}
