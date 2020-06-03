#include "FixParser.h"

#include "FixMessage.h"
#include "ParseUtils.h"

namespace fix_parser {
    void FixParser::process(const std::string &filename) {
        _reader->load(filename);
    }

    std::tuple<int, std::string_view, std::string_view> FixParser::getNextTagValue(const std::string_view &line) {
        size_t pos_value_delimeter = line.find(TAG_VALUE_DELIMETER);
        std::tuple<int, std::string_view, std::string_view> res{};

        if (pos_value_delimeter == std::string_view::npos) {
            std::cerr << "Failed to find a value delimeter in line: " << line << std::endl;
            return res;
        }

        size_t pos_field_delimeter = line.find(DELIMITER, pos_value_delimeter);
        if (pos_field_delimeter == std::string_view::npos) {
            std::cerr << "Failed to find line field delimeter in line: " << line << std::endl;
            return res;
        }

        auto[tag, ec] = parseNumber<int>(line.data(), line.data() + pos_value_delimeter);
        if (ec != std::errc()) {
            std::cerr << "Failed to parse tag in line: " << line << std::endl;
            return res;
        }

        size_t valueLen = pos_field_delimeter - pos_value_delimeter - 1;

        constexpr int VALUE_DELIMETER_LEN = 1;
        constexpr int FIELD_DELIMETER_LEN = 1;

        std::get<0>(res) = tag;
        std::get<1>(res) = std::string_view(line.data() + pos_value_delimeter + VALUE_DELIMETER_LEN, valueLen);
        std::get<2>(res) = std::string_view(line.data() + pos_field_delimeter + FIELD_DELIMETER_LEN);

        return res;
    }

    std::string_view FixParser::parse_group(const std::string_view &groupStr, const std::string_view &numEntriesStr,
                                            const std::string_view &msgType,
                                            const std::string_view &line, uint64_t volumeMultiplier) {
        auto[numMDEntries, ec] = parseNumber<int>(numEntriesStr);
        if (ec != std::errc()) {
            std::cerr << "Failed to parse numMDEntries tag value: " << numEntriesStr << std::endl;
            return groupStr;
        }

        std::string_view strLeft = groupStr;
        for (size_t i = 0; i < numMDEntries; ++i) {
            FixMessage msg;
            if (msgType == MARKET_DATA_SNAPSHOT_TYPE) {
                msg.setUpdateAction("0"); // all msgs in MDEntries group have 'New' MDUpdateAction by default
            }

            while (!msg.isCompleted() && !strLeft.empty()) {
                auto[tag, value, updatedStr] = getNextTagValue(strLeft);
                strLeft = updatedStr;
                switch (tag) {
                    case MD_ENTRY_TYPE_TAG:
                        msg.setSide(value);
                        break;
                    case MD_ENTRY_PX:
                        msg.setPrice(value);
                        break;
                    case MD_ENTRY_SIZE:
                        msg.setQty(value, volumeMultiplier);
                        break;
                    case MD_UPDATE_ACTION_TAG:
                        msg.setUpdateAction(value);
                        break;
                    default:
                        break;
                }
            }

            if (!msg.isCompleted()) {
                std::cerr << "Failed to parse " << i << "/" << numMDEntries << " MDEntry group msg in line: " << line
                          << std::endl;
                break;
            }

            FixMessage::UpdateAction ua = msg.getUpdateAction().value();
            LevelData levelData;
            levelData.price = msg.getPrice().value();
            levelData.qty = msg.getQty().value();
            levelData.side = msg.getSide().value();
            if (ua == FixMessage::UpdateAction::Delete) {
                _order_book.del(levelData);
            } else {
                _order_book.add_change(levelData);
            }
        }

        return strLeft;
    }

    void FixParser::handle_line(const std::string &line) {
        std::string_view lineStr = line.c_str();
        std::string_view msgType{};
        uint64_t contractSizeMultiplier{1};
        while (!lineStr.empty()) {
            auto[tag, value, strLeft] = getNextTagValue(lineStr);
            lineStr = strLeft;

            switch (tag) {
                case MSG_TYPE_TAG: {
                    if (value != MARKET_DATA_SNAPSHOT_TYPE &&
                        value != MARKET_DATA_INCREMENTAL_REFRESH_TYPE) {
                        std::cerr << "Skipping unknown message, type tag: " << value << std::endl;
                        return;
                    }
                    msgType = value;
                    break;
                }
                case CONTRACT_MULTIPLIER_TAG: {
                    auto[multiplier, ec] = parseNumber<uint64_t>(value);
                    if (ec != std::errc()) {
                        std::cerr << "Failed to parse ContractMultiplier tag value: " << value << std::endl;
                        break;
                    }
                    contractSizeMultiplier = multiplier;
                    break;
                }
                case NO_MD_ENTRIES:
                    lineStr = parse_group(lineStr, value, msgType, line, contractSizeMultiplier);
                    break;
                default:
                    break;
            }
        }

        std::cout << market_data_to_s(_orderBookDepth) << std::endl;
    }

    std::string FixParser::market_data_to_s(const size_t depth) {
        return _order_book.to_s(depth);
    }

}
