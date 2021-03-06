#include <FixParser.h>

#include <ParseUtils.h>

namespace fix_parser {
    void FixParser::process(const std::string &filename) {
        _reader->load(filename);
    }

    std::tuple<int, std::string_view, std::string_view> FixParser::get_next_tag_value(const std::string_view &line) {
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

        auto[tag, ec] = parse_number<int>(line.data(), line.data() + pos_value_delimeter);
        if (ec != std::errc()) {
            std::cerr << "Failed to parse tag in line: " << line << std::endl;
            return res;
        }

        size_t value_len = pos_field_delimeter - pos_value_delimeter - 1;

        constexpr int VALUE_DELIMETER_LEN = 1;
        constexpr int FIELD_DELIMETER_LEN = 1;

        std::get<0>(res) = tag;
        std::get<1>(res) = std::string_view(line.data() + pos_value_delimeter + VALUE_DELIMETER_LEN, value_len);
        std::get<2>(res) = std::string_view(line.data() + pos_field_delimeter + FIELD_DELIMETER_LEN);

        return res;
    }

    std::tuple<bool, bool>
    FixParser::parse_first_group_required_tag(FixMessage &msg, int tag, const std::string_view &value,
                                              const std::string_view &msg_type) {
        if (msg_type == MARKET_DATA_INCREMENTAL_REFRESH_TYPE) {
            if (tag != MD_UPDATE_ACTION_TAG) {
                std::cerr << "Failed to parse MDIncrementalRefresh message,"
                             " mandatory tag MDUpdateAction in MDEntries group is missing" << std::endl;
                return {true, false};
            }

            if (!msg.set_update_action(value)) {
                return {false, true};
            }
        } else if (msg_type == MARKET_DATA_SNAPSHOT_TYPE) {
            if (tag != MD_ENTRY_TYPE_TAG) {
                std::cerr << "Failed to parse MDSnapshot message, mandatory tag MDEntryType "
                             "in MDEntries group is missing" << std::endl;
                return {true, false};
            }

            if (!msg.set_side(value)) {
                return {false, true};
            }
            msg.set_update_action("0"); // all msgs in MDEntries group have 'New' MDUpdateAction by default
        } else {
            return {true, false};
        }

        return {false, false};
    }

    bool FixParser::mandatory_tag_occurred(std::string_view &mandatory_value, int tag, const std::string_view &value,
                                           const std::string_view &msg_type) {
        if (msg_type == MARKET_DATA_INCREMENTAL_REFRESH_TYPE && tag == MD_UPDATE_ACTION_TAG) {
            mandatory_value = value;
            return true;
        } else if (msg_type == MARKET_DATA_SNAPSHOT_TYPE && tag == MD_ENTRY_TYPE_TAG) {
            mandatory_value = value;
            return true;
        }

        return false;
    }

    void FixParser::update_order_book(const FixMessage &msg) {
        FixMessage::UpdateAction ua = msg.get_update_action().value();
        LevelData level_data;
        level_data.price = msg.get_price().value();
        level_data.qty = msg.get_qty().value();
        level_data.side = msg.get_side().value();
        if (ua == FixMessage::UpdateAction::Delete) {
            _order_book.del(level_data);
        } else {
            _order_book.add_change(level_data);
        }
    }

    std::string_view FixParser::parse_md_group(const std::string_view &msg_type, const std::string_view &group,
                                               const std::string_view &num_entries_str, uint64_t volume_multiplier) {
        std::string_view str_left = group;
        auto[num_md_entries, ec] = parse_number<int>(num_entries_str);
        if (ec != std::errc()) {
            std::cerr << "Failed to parse numMDEntries tag value: " << num_entries_str << std::endl;
            return str_left;
        }

        auto[mandatory_tag, mandatory_value, updated_str] = get_next_tag_value(str_left);
        str_left = updated_str;
        for (size_t i = 0; i < num_md_entries; ++i) {
            FixMessage msg;

            if (str_left.empty()) {
                break;
            }

            auto[err_occured, to_continue] = parse_first_group_required_tag(msg, mandatory_tag, mandatory_value,
                                                                            msg_type);
            if (err_occured) {
                return str_left;
            }

            if (to_continue) {
                continue;
            }

            while (!str_left.empty()) {
                auto[sub_tag, sub_value, sub_updated_str] = get_next_tag_value(str_left);
                str_left = sub_updated_str;
                if (mandatory_tag_occurred(mandatory_value, sub_tag, sub_value, msg_type)) {
                    break;
                }
                if (sub_tag == MD_UPDATE_ACTION_TAG) {
                    msg.set_update_action(sub_value);
                } else if (sub_tag == MD_ENTRY_PX) {
                    msg.set_price(sub_value);
                } else if (sub_tag == MD_ENTRY_SIZE) {
                    msg.set_qty(sub_value, volume_multiplier);
                } else if (sub_tag == MD_ENTRY_TYPE_TAG) {
                    msg.set_side(sub_value);
                }
            }

            if (msg.is_completed()) {
                update_order_book(msg);
            }
        }

        return str_left;
    }

    void FixParser::parse_md_msg(const std::string_view &msg, const std::string_view &msg_type) {
        uint64_t contract_size_multiplier{1};
        std::string_view line = msg;
        while (!line.empty()) {
            auto[tag, value, line_left] = get_next_tag_value(line);
            line = line_left;

            switch (tag) {
                case CONTRACT_MULTIPLIER_TAG: {
                    auto[multiplier, ec] = parse_number<uint64_t>(value);
                    if (ec != std::errc()) {
                        std::cerr << "Failed to parse ContractMultiplier tag value: " << value << std::endl;
                        break;
                    }
                    contract_size_multiplier = multiplier;
                    break;
                }
                case NO_MD_ENTRIES: {
                    parse_md_group(msg_type, line, value, contract_size_multiplier);
                    return;
                }
                default:
                    break;
            }
        }
    }

    void FixParser::handle_line(const std::string &line) {
        std::string_view line_str = line.c_str();
        while (!line_str.empty()) {
            auto[tag, value, str_left] = get_next_tag_value(line_str);
            line_str = str_left;

            if (tag != MSG_TYPE_TAG) {
                continue;
            }

            if (value == MARKET_DATA_SNAPSHOT_TYPE || value == MARKET_DATA_INCREMENTAL_REFRESH_TYPE) {
                parse_md_msg(line_str, value);
                break;
            } else {
                std::cerr << "Skipping message, type tag: " << value << std::endl;
                return;
            }
        }

        std::cout << market_data_to_s(_orderBookDepth) << std::endl;
    }

    std::string FixParser::market_data_to_s(const size_t depth) {
        return _order_book.to_s(depth);
    }

}
