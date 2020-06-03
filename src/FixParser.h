#pragma once

#include <DataReader.h>
#include <OrderBook.h>

#include <memory>

namespace fix_parser {
    class FixParser : public DataReaderHandler {
    public:
        FixParser(size_t orderBookDepth = 5) : _orderBookDepth(orderBookDepth),
                                           _reader(std::make_unique<DataReader>(this)) {};
        // specified 'orderBookDepth' parameter defines depth of OrderBook to print after each processed message

        void handle_line(const std::string &line) override;
        // overridden callback which is intended to be called by DataReader on each tick's 'line'

        void process(const std::string &filename);
        // proceeds ticks from the specified 'filename'

        std::string market_data_to_s(const size_t depth);
        // print to stdout current OrderBook with specified 'depth'

    private:
        std::tuple<int, std::string_view, std::string_view> getNextTagValue(const std::string_view &line);

        std::string_view
        parse_group(const std::string_view& groupStr, const std::string_view &numEntriesStr, const std::string_view &msgType,
                    const std::string_view &line, uint64_t volumeMultiplier);

        const char DELIMITER{'^'};
        const char TAG_VALUE_DELIMETER{'='};
        const std::string_view MARKET_DATA_SNAPSHOT_TYPE{"W"};
        const std::string_view MARKET_DATA_INCREMENTAL_REFRESH_TYPE{"X"};

        constexpr static int MSG_TYPE_TAG{35};
        constexpr static int CONTRACT_MULTIPLIER_TAG{231};
        constexpr static int NO_MD_ENTRIES{268};
        constexpr static int MD_ENTRY_TYPE_TAG{269};
        constexpr static int MD_ENTRY_PX{270};
        constexpr static int MD_ENTRY_SIZE{271};
        constexpr static int MD_UPDATE_ACTION_TAG{279};

        const size_t _orderBookDepth;
        std::unique_ptr<DataReader> _reader;
        OrderBook _order_book;
    };
}
