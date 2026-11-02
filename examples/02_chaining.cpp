#include "port.hpp"

#include <iostream>

#if __cpp_lib_expected < 202211L
#error Support for std::expected monadic operations is required (202211L).
#endif

std::expected<int, std::string> require_high_port(int port) {
    if (port < 1024) {
        return std::unexpected("the application requires a port >= 1024");
    }
    return port;
}

int main() {
    for (std::string_view text : {"8080", "80", "abc"}) {
        auto address = parse_port(text)
            .and_then(require_high_port)
            .transform([](int port) {
                return "localhost:" + std::to_string(port);
            });

        if (address) {
            std::cout << *address << '\n';
        } else {
            std::cout << "Error: " << address.error() << '\n';
        }
    }

    auto described = parse_port("abc").transform_error([](const std::string& e) {
        return "Configuration: " + e;
    });
    if (!described) {
        std::cout << described.error() << '\n';
    }

    auto recovered = parse_port("abc").or_else(
        [](const std::string&) -> std::expected<int, std::string> {
            return 8080;
        });
    if (recovered) {
        std::cout << "After recovery: " << *recovered << '\n';
    }
}
