#pragma once

#include <charconv>
#include <expected>
#include <string>
#include <string_view>
#include <system_error>

#if !defined(__cpp_lib_expected) || __cpp_lib_expected < 202202L
#error A standard library with std::expected support is required (C++23).
#endif

inline std::expected<int, std::string> parse_port(std::string_view text) {
    if (text.empty()) {
        return std::unexpected("empty input");
    }

    int port{};
    const char* end = text.data() + text.size();
    auto [ptr, ec] = std::from_chars(text.data(), end, port);

    if (ec == std::errc::invalid_argument || ptr != end) {
        return std::unexpected("expected an integer");
    }
    if (ec == std::errc::result_out_of_range || port < 1 || port > 65535) {
        return std::unexpected("port out of range 1..65535");
    }
    return port;
}
