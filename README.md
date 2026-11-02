# std::expected — materiały do zajęć

[Materiały dla słuchaczy](materialy.md) wyjaśniają, po co używać `std::expected`, jak obsługiwać sukces i błąd oraz jak łączyć operacje.

Przykłady:

- [01_basics.cpp](examples/01_basics.cpp) — parsowanie portu, obsługa błędów, wartość domyślna i `expected<void, E>`.
- [02_chaining.cpp](examples/02_chaining.cpp) — `and_then`, `transform`, `transform_error` i `or_else`.
- [port.hpp](examples/port.hpp) — wspólny parser portu.

Wymagane są C++23 i biblioteka standardowa z obsługą `std::expected`; drugi przykład wymaga także operacji monadycznych. Instrukcje kompilacji dla GCC, Clang i MSVC znajdują się w [materiale](materialy.md#10-jak-uruchomić-przykłady).

Przykłady sprawdzono przy użyciu MSVC, toolset 14.41.34120, na Windows ARM64.
