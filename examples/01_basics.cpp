#include "port.hpp"

#include <iostream>

std::expected<std::string, std::string> make_address(std::string_view text) {
    auto port = parse_port(text);
    if (!port) {
        return std::unexpected(port.error());
    }
    return "localhost:" + std::to_string(*port);
}

std::expected<void, std::string> validate_name(std::string_view name) {
    if (name.empty()) {
        return std::unexpected("name is empty");
    }
    return {};
}

int main() {
    for (std::string_view text : {"8080", "abc", "70000", "80xyz", ""}) {
        auto result = parse_port(text);
        std::cout << '"' << text << "\": ";
        if (result) {
            std::cout << "port = " << *result << '\n';
        } else {
            std::cout << "error: " << result.error() << '\n';
        }
    }

    std::cout << "Default port: " << parse_port("abc").value_or(8080) << '\n';
    for (std::string_view text : {"8080", "abc"}) {
        auto address = make_address(text);
        if (address) {
            std::cout << "Address: " << *address << '\n';
        } else {
            std::cout << "Address error: " << address.error() << '\n';
        }
    }
    for (std::string_view name : {"Alice", ""}) {
        auto result = validate_name(name);
        if (result) {
            std::cout << "Valid name\n";
        } else {
            std::cout << result.error() << '\n';
        }
    }

    std::expected<int, std::string> zero;
    std::cout << "Success with zero: " << std::boolalpha << zero.has_value()
              << ", value: " << *zero << '\n';
    try {
        auto invalid = parse_port("abc");
        std::cout << invalid.value() << '\n';
    } catch (const std::bad_expected_access<std::string>& e) {
        std::cout << "value() threw an exception: " << e.error() << '\n';
    }
}
