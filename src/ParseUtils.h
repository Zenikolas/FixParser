#pragma once

#include <tuple>
#include <charconv>

namespace fix_parser {

template <class Number, class = std::enable_if_t<std::is_integral_v<Number>>>
std::tuple<Number, std::errc> parse_number(const std::string_view& str)
{
    Number ret{};
    auto[_, ec] = std::from_chars(str.data(), str.data() + str.size(), ret);
    return {ret, ec};
}

template <class Number, class = std::enable_if_t<std::is_integral_v<Number>>>
std::tuple<Number, std::errc> parse_number(const char* begin, const char* end)
{
    Number ret{};
    auto[_, ec] = std::from_chars(begin, end, ret);
    return {ret, ec};
}

}
