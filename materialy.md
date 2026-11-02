# std::expected — materiały dla słuchaczy (C++23)

`std::expected<T, E>` opisuje wynik operacji: **sukces z wartością typu `T` albo błąd typu `E`**. Dzięki temu sygnatura funkcji pokazuje zarówno typ wyniku, jak i sposób zgłaszania niepowodzenia.

## 1. Jaki problem rozwiązujemy?

Funkcja wczytująca numer portu z tekstu może dostać `"8080"`, ale też `"abc"` albo `"70000"`. Nie każde wywołanie zakończy się więc uzyskaniem poprawnego portu. Ta możliwość powinna być widoczna dla osoby korzystającej z funkcji.

Jeśli funkcja zwraca `int`, a błąd sygnalizuje liczbą `-1`, sama sygnatura nie pokazuje tej umowy:

```cpp
int parse_port(std::string_view text);
```

Z punktu widzenia typu wynik jest zwykłą liczbą. Możemy od razu przekazać go dalej jako port, zapominając, że najpierw należało sprawdzić `-1`. Żeby wiedzieć o możliwości błędu i sposobie jego rozpoznania, trzeba zajrzeć do dokumentacji lub implementacji.

Z `std::expected` zapisujemy tę informację w typie wyniku:

```cpp
std::expected<int, std::string> parse_port(std::string_view text);
//            ^   ^
//          wynik opis błędu
```

**Sygnatura mówi wprost: funkcja może zwrócić port albo błąd. Wywołujący musi zdecydować, co zrobić w obu przypadkach, żeby poprawnie wykorzystać wynik.** Przy sukcesie może użyć portu. Przy błędzie może wyświetlić komunikat, poprosić o ponowne podanie danych, wybrać wartość domyślną albo przekazać błąd dalej.

Otrzymany obiekt nie jest zwykłym `int` i nie konwertuje się do niego automatycznie. Najpierw trzeba wybrać sposób uzyskania wartości: sprawdzić stan i ją odczytać, podać wartość zastępczą lub użyć dostępu, który przy błędzie rzuci wyjątek. Te możliwości omówimy w kolejnych rozdziałach.

Osobny typ błędu pozwala dodatkowo przekazać przyczynę niepowodzenia, np. odróżnić pusty tekst od przekroczenia zakresu. Najważniejsze jest jednak to, że możliwość błędu stanowi część interfejsu funkcji, a decyzja o reakcji należy do wywołującego.

| Sposób | Co przekazuje? | Przykładowe zastosowanie |
|---|---|---|
| `std::optional<T>` | wartość albo jej brak | wyszukiwanie elementu, gdy brak nie wymaga wyjaśnienia |
| `std::expected<T, E>` | wartość albo konkretny błąd | parsowanie i walidacja danych |
| wyjątek | informację o błędzie do pasującego `catch` | obsługa błędu na wyższym poziomie wywołań |

Wybór zależy od kontraktu funkcji i zasad projektu. `expected` wymaga uwzględnienia możliwości błędu przy poprawnym korzystaniu z wyniku, ale nie wymusza jego obsługi na poziomie kompilatora: programista nadal może zignorować cały wynik albo błędnie odczytać wartość bez sprawdzenia stanu.

W porównaniu z wyjątkami zmienia się sposób przekazywania niepowodzenia. `return std::unexpected(...)` kończy bieżącą funkcję i oddaje wynik bezpośrednio wywołującemu. Ten może obsłużyć błąd albo zwrócić go dalej. Wyjątek szuka pasującego `catch`, także w dalszych wywołaniach. `expected` przydaje się więc tam, gdzie chcemy podejmować decyzję o błędzie jawnie przy kolejnych etapach operacji; wyjątki pozwalają skupić obsługę na wyższym poziomie.

## 2. Dwa stany i sposób ich tworzenia

Wymagany jest nagłówek `<expected>` i biblioteka standardowa obsługująca tę funkcję C++23. `T` oznacza typ wartości sukcesu, `E` — typ błędu. Nie muszą to być liczba i napis; błąd może być np. wyliczeniem lub strukturą z kodem i opisem.

```cpp
#include <expected>
#include <string>

std::expected<int, std::string> ok = 8080;
std::expected<int, std::string> fail = std::unexpected("invalid port");
```

`std::unexpected(...)` oznacza: „to jest błąd”. To obiekt, **nie instrukcja rzucenia wyjątku**. Z funkcji o odpowiednim typie wyniku zwracamy więc:

```cpp
return 8080;                              // sukces
// albo:
return std::unexpected("invalid port");   // błąd
```

Te dwa `return` pokazują alternatywne ścieżki funkcji. Kompletny przykład znajduje się w rozdziale 4.

`T` i `E` mogą być takie same:

```cpp
std::expected<std::string, std::string> a = "ready"; // sukces
std::expected<std::string, std::string> b =
    std::unexpected("no data");                 // błąd
```

Ten przykład wyjaśnia, po co osobne `std::unexpected`. Sam napis nie wystarcza do rozpoznania intencji: zarówno poprawny wynik, jak i opis błędu mają tutaj typ `std::string`. Zwykłe przypisanie wartości tworzy sukces, a opakowanie jej w `unexpected` jednoznacznie wybiera stan błędu. Biblioteka nie zgaduje tego na podstawie treści napisu.

Obiekt reprezentuje jeden z tych stanów. Nie ma osobnego stanu „ani wartość, ani błąd”. Zasady modelu opisuje [specyfikacja `expected`](https://eel.is/c++draft/expected.general).

To różni go od struktury z niezależnymi polami `value` i `error`. Taka struktura mogłaby zawierać jednocześnie wynik i komunikat błędu, a użytkownik musiałby wiedzieć, któremu polu ufać. W `expected` stan rozstrzyga, która część jest dostępna. Nie odczytujemy obu i nie szukamy w jednej z nich umownej wartości oznaczającej brak danych.

Typ błędu dobieramy do tego, co wywołujący ma z nim zrobić. Napis jest wygodny do wyświetlenia człowiekowi. Gdy program ma reagować inaczej na pusty tekst, zły format i przekroczony zakres, wygodniejsze może być `enum class ParseError { empty, invalid_format, out_of_range };`. Wtedy można użyć `switch` zamiast porównywać treść komunikatów.

## 3. Jak odczytać wynik?

```cpp
if (ok) {                       // równoważne ok.has_value()
    std::cout << *ok << '\n';
} else {
    std::cout << ok.error() << '\n';
}
```

Warunek sprawdza **stan**, a nie zawartość wyniku. Sukces z liczbą `0`, a nawet sukces z `false` w `expected<bool, E>`, nadal daje prawdziwy warunek `if (result)`.

Poprawne wykonanie operacji i wartość jej wyniku to dwie różne informacje. Funkcja sprawdzająca istnienie pliku może poprawnie ustalić, że pliku nie ma — wtedy sukces zawiera `false`. Błąd oznaczałby, że nie udało się tego ustalić, np. z powodu braku dostępu. `if (result)` odpowiada na pytanie „czy operacja się udała?”, a dopiero `*result` daje jej odpowiedź.

| Wyrażenie | Znaczenie / warunek użycia |
|---|---|
| `r.has_value()` / `if (r)` | sprawdza sukces |
| `*r`, `r->field` | dostęp do wartości; wymaga sukcesu |
| `r.error()` | dostęp do błędu; wymaga stanu błędu |
| `r.value()` | zwraca wartość; przy błędzie rzuca `std::bad_expected_access<E>` |
| `r.value_or(default_value)` | zwraca wartość lub podaną wartość zastępczą |

`*r` i `.error()` nie są zamiennikami sprawdzenia stanu. W C++23 użycie ich w niewłaściwym stanie powoduje niezdefiniowane zachowanie. `.value()` sprawdza stan i może rzucić wyjątek. Zasady dostępu określa [specyfikacja obserwatorów C++23](https://timsong-cpp.github.io/cppwp/n4950/expected.object.obs).

Te sposoby dostępu odpowiadają różnym założeniom w kodzie. W gałęzi `if (r)` wiemy już, że wartość istnieje, więc możemy użyć `*r`. Wywołanie `.value()` oznacza: „oczekuję wartości, a jeśli jej nie ma, zgłoś to wyjątkiem”. Dlatego zastąpienie każdego sprawdzenia przez `.value()` zmieni sposób obsługi błędów — zamiast samemu zareagować na zwrócony błąd, przejdziemy na ścieżkę wyjątków.

## 4. Pełny przykład: numer portu z tekstu

Przyjmujemy dziesiętną liczbę od **1 do 65535**, bez spacji i znaku `+`. Zera wiodące są dozwolone. To reguły naszego parsera; port 0 odrzucamy jako decyzję aplikacji.

```cpp
#include <charconv>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>

std::expected<int, std::string> parse_port(std::string_view text) {
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
}
```

Wynik:

```text
"8080": port = 8080
"abc": error: expected an integer
"70000": error: port out of range 1..65535
"80xyz": error: expected an integer
"": error: empty input
```

`std::from_chars` dostarcza wskaźnik końca odczytanej liczby (`ptr`) i kod błędu (`ec`). Sprawdzenie `ptr != end` odrzuca tekst `"80xyz"`: odczytanie początkowego `80` nie wystarcza do zaakceptowania całego wejścia. Dokumentacja pierwotna: [`from_chars`](https://timsong-cpp.github.io/cppwp/n4950/charconv.from.chars).

Zwróć uwagę na podział odpowiedzialności: parser **zwraca** błąd, a `main` decyduje o jego wypisaniu. W innej aplikacji wywołujący mógłby poprosić o ponowne wprowadzenie danych.

Gdyby parser sam wypisywał komunikaty, jego użycie w oknie aplikacji albo w testach powodowałoby również niechciane pisanie na konsolę. Zwrócenie błędu pozwala użyć tej samej funkcji w różnych miejscach: interfejs wyświetli komunikat, program wsadowy zapisze go w logu, a test sprawdzi otrzymany wynik.

## 5. Wartość domyślna i przekazanie błędu dalej

Jeśli błąd ma oznaczać użycie domyślnego portu:

```cpp
int port = parse_port("abc").value_or(8080); // 8080
```

Ten zapis pomija przyczynę błędu. Stosuj go wtedy, gdy takie zachowanie wynika z wymagań.

Brak opcjonalnego ustawienia może uzasadniać użycie domyślnego portu. Literówka w porcie podanym przez użytkownika może natomiast wymagać komunikatu i poprawienia konfiguracji. `value_or` potraktuje każdy błąd tak samo, więc nie zawsze odpowiada temu, co aplikacja powinna zrobić.

Jeżeli kolejna funkcja też zwraca `expected`, może przekazać błąd wyżej:

```cpp
std::expected<std::string, std::string> make_address(std::string_view text) {
    auto port = parse_port(text);
    if (!port) {
        return std::unexpected(port.error());
    }
    return "localhost:" + std::to_string(*port);
}
```

Wyniki: dla `"8080"` sukces z `"localhost:8080"`, dla `"abc"` błąd z komunikatem parsera. Sam obiekt `port` ma inny typ sukcesu niż wynik `make_address`, więc jawnie opakowujemy jego błąd w `std::unexpected`.

`make_address` wie, jak z poprawnego portu utworzyć adres, ale nie wie, jak naprawić błędne wejście. Przekazuje więc przyczynę niepowodzenia wywołującemu. W ten sposób funkcje pośrednie mogą zakończyć pracę przy pierwszym błędzie, a decyzję o ponowieniu operacji lub wyświetleniu komunikatu zostawić miejscu, które ma potrzebny kontekst.

## 6. Łączenie operacji — rozszerzenie

Przy wielu etapach można zastąpić kolejne warunki metodami nazywanymi **operacjami monadycznymi**. Nazwa oznacza tu praktyczne składanie operacji z uwzględnieniem sukcesu i błędu; nie trzeba znać teorii monad.

Motywację widać w `make_address`: po wywołaniu parsera sprawdzamy stan, przekazujemy błąd i dopiero wtedy przetwarzamy wartość. Przy trzech lub czterech etapach ten sam schemat powtarzałby się kilka razy. Metody poniżej zapisują go raz w bibliotece, a w naszym kodzie pozwalają pokazać kolejność właściwych operacji.

| Metoda | Kiedy wywołuje funkcję? | Co ma zwrócić funkcja? |
|---|---|---|
| `transform(f)` | przy sukcesie, przekazuje wartość | nową wartość; typ błędu pozostaje ten sam |
| `and_then(f)` | przy sukcesie, przekazuje wartość | `expected<U, E>` z tym samym typem błędu |
| `transform_error(f)` | przy błędzie, przekazuje błąd | nowy błąd; typ sukcesu pozostaje ten sam |
| `or_else(f)` | przy błędzie, przekazuje błąd | `expected<T, G>` z tym samym typem sukcesu |

Pozostała ścieżka przechodzi dalej bez wywołania `f`. Wynik metody jest nowym obiektem `expected`. Metody te należą do C++23; starsza implementacja biblioteki może obsługiwać sam `expected` bez nich. Szczegóły: [operacje monadyczne C++23](https://timsong-cpp.github.io/cppwp/n4950/expected.object.monadic), [propozycja P2505R5](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2505r5.html).

Przykład z dodatkową regułą aplikacji: akceptujemy tylko porty od 1024.

```cpp
std::expected<int, std::string> require_high_port(int port) {
    if (port < 1024) {
        return std::unexpected("the application requires a port >= 1024");
    }
    return port;
}

auto address = parse_port("8080")
    .and_then(require_high_port)
    .transform([](int port) {
        return "localhost:" + std::to_string(port);
    });
// Typ address: std::expected<std::string, std::string>
// Sukces: "localhost:8080"
```

Dla `"80"` drugi etap zwróci błąd i lambda z `transform` zostanie pominięta. Dla `"abc"` błąd powstanie już w parserze, więc oba dalsze etapy zostaną pominięte. `and_then` służy tu do operacji, która sama może zwrócić błąd; `transform` do utworzenia napisu z poprawnego portu.

Rozdzielenie tych metod wynika z typu zwracanego przez przekazaną funkcję. Lambda tworząca adres zwraca `std::string`, więc `transform` opakowuje ten napis jako nową wartość sukcesu. `require_high_port` zwraca już cały `expected`, bo może odrzucić port. `and_then` przyjmuje ten wynik jako wynik kolejnego etapu. Dzięki temu nadal mamy jeden stan sukcesu lub błędu, zamiast obiektu `expected` przechowującego następny `expected`.

Pozostałe metody na tym samym parserze:

```cpp
auto described = parse_port("abc").transform_error([](const std::string& e) {
    return "Configuration: " + e;
}); // nadal błąd, z dodatkowym kontekstem

auto recovered = parse_port("abc").or_else(
    [](const std::string&) -> std::expected<int, std::string> {
        return 8080;
    }); // sukces z 8080
```

`transform_error` przekształca błąd. `or_else` może zamienić go w sukces albo zwrócić kolejny błąd. Pełny uruchamialny wariant: [02_chaining.cpp](examples/02_chaining.cpp).

## 7. Sukces bez wartości: `std::expected<void, E>`

Nie każda operacja ma wynik do zwrócenia. Walidacja może jedynie zakończyć się sukcesem lub podać błąd:

```cpp
std::expected<void, std::string> validate_name(std::string_view name) {
    if (name.empty()) {
        return std::unexpected("name is empty");
    }
    return {}; // sukces, bez wartości
}

auto result = validate_name("Alice");
if (!result) {
    std::cout << result.error() << '\n';
}
```

`void` zajmuje miejsce typu sukcesu. Można sprawdzać stan i odczytywać błąd. Nie ma wartości do wypisania ani metody `value_or`. Szczegóły: [specjalizacja dla `void`](https://timsong-cpp.github.io/cppwp/n4950/expected.void).

Zwrócenie samego `bool` powiedziałoby jedynie, czy walidacja się udała. Zwykłe `void` nie przeniosłoby informacji o błędzie przez wynik funkcji. `expected<void, E>` pozwala zachować opis niepowodzenia bez wymyślania wartości sukcesu, np. liczby `0`, która nie wnosiłaby żadnej informacji.

## 8. Typowe pułapki

**Domyślna konstrukcja oznacza sukces.**

```cpp
std::expected<int, std::string> r;
// r.has_value() == true, *r == 0
```

Domyślny konstruktor jest dostępny, jeśli można domyślnie skonstruować `T`; inicjalizuje wartość sukcesu. To inne zachowanie niż pusty `std::optional<int>`. Źródło: [konstruktory C++23](https://timsong-cpp.github.io/cppwp/n4950/expected.object.cons).

**`expected` nie oznacza automatycznie „bez wyjątków”.** `.value()` może rzucić wyjątek. Może go też rzucić np. konstruktor przechowywanego napisu lub funkcja wywołana w `transform`. Typ wyniku sam nie czyni funkcji `noexcept`.

**`value_or` nie oblicza argumentu leniwie.** W `r.value_or(compute_default())` wywołanie `compute_default()` następuje przed wejściem do metody, także przy sukcesie. Jeśli obliczenie jest kosztowne lub ma efekty uboczne, użyj warunku lub odpowiednio `or_else`.

**`transform` i `and_then` mają różne kontrakty.** Funkcję zwracającą `expected` zwykle łączymy przez `and_then`. Przekazanie jej do `transform` tworzy zagnieżdżony `expected<expected<...>, E>`.

## 9. Ściąga

```cpp
// Typ zwracany:
std::expected<T, E>

// Zwrócenie sukcesu / błędu:
return value;
return std::unexpected(error);

// Obsługa wyniku:
if (r) { use(*r); }
else   { handle(r.error()); }

// Sukces bez wartości:
std::expected<void, E>
return {};
```

To schematy składni — nazwy `T`, `E`, `value`, `error`, `use` i `handle` zastępujesz elementami własnego programu.

Zapamiętaj: `expected` przechowuje **wartość albo błąd**, `unexpected` oznacza ścieżkę błędu, a przed użyciem `*r` lub `r.error()` sprawdzasz odpowiedni stan.

## 10. Jak uruchomić przykłady?

Z katalogu `expected`, przy dostępnych GCC lub Clang i odpowiedniej bibliotece standardowej:

```sh
g++ -std=c++23 -Wall -Wextra -pedantic examples/01_basics.cpp -o basics
g++ -std=c++23 -Wall -Wextra -pedantic examples/02_chaining.cpp -o chaining
```

Uruchom `./basics` i `./chaining` (na Windowsie odpowiednie pliki `.exe`). Dla Clang zamień `g++` na `clang++`.

W Developer Command Prompt for Visual Studio, z tego samego katalogu:

```bat
cl /nologo /std:c++latest /EHsc /W4 /utf-8 examples\01_basics.cpp /Fe:basics.exe
cl /nologo /std:c++latest /EHsc /W4 /utf-8 examples\02_chaining.cpp /Fe:chaining.exe
basics.exe
chaining.exe
```

Flaga wyboru standardu nie zastępuje wsparcia biblioteki. W programach dodano sprawdzenie makra `__cpp_lib_expected`: co najmniej `202202L` dla podstaw i `202211L` dla operacji monadycznych. Drugi przykład zgłosi czytelny błąd kompilacji przy częściowej obsłudze. Nieobecny nagłówek `<expected>` wymaga aktualizacji lub wyboru innej biblioteki standardowej. [P2505R5 — makro wykrywania funkcji](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2505r5.html#feature-test-macro).

## 11. Źródła i dalsza lektura

- [Projekt C++23 N4950: `std::expected`](https://timsong-cpp.github.io/cppwp/n4950/expected) — punkt odniesienia dla wersji standardu omawianej na zajęciach; odsyłacze do konkretnych sekcji zamieszczono przy wyjaśnieniach.
- [WG21 P0323R12: `std::expected`](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0323r12.html) — motywacja i projekt narzędzia.
- [WG21 P2505R5: operacje monadyczne](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2505r5.html) — uzasadnienie interfejsu łączenia operacji.
