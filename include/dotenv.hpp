#pragma once

#include <charconv>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace dotenv {
struct ThrowPolicy {};
struct NoThrowPolicy {};

inline void errcToException(std::errc ec, std::string_view value) {
    switch (ec) {
    case std::errc::invalid_argument:
        throw std::invalid_argument("Invalid argument: " + std::string(value));
    case std::errc::result_out_of_range:
        throw std::out_of_range("Result out of range: " + std::string(value));
    default:
        throw std::runtime_error(std::string("Unknown error: ") +
                                 std::to_string(int(ec)) + " " +
                                 std::string(value));
    }
}

int load(std::string_view path = ".env", int replace = 1) noexcept;
std::string_view get(std::string_view key, std::string_view default_value = "");

template <class T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T get(std::string_view key,
             std::optional<T> default_value = std::nullopt) {
    auto value = get(key, std::string_view{""});

    if (value.empty()) {
        return default_value.value();
    }

    T result{};
    auto [ptr, ec] =
        std::from_chars(value.data(), value.data() + value.size(), result);

    if (ec != std::errc{}) {
        if (default_value.has_value()) {
            return default_value.value();
        }

        errcToException(ec, value);
    }

    return result;
}

template <class T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T get([[maybe_unused]] NoThrowPolicy policy, std::string_view key,
             T default_value = {}) noexcept {
    auto value = get(key, std::string_view{});

    if (value.empty()) {
        return default_value;
    }

    T result{};
    auto [ptr, ec] =
        std::from_chars(value.data(), value.data() + value.size(), result);

    // Check for parsing errors or out-of-range values
    if (ec != std::errc{}) {
        return default_value;
    }

    return result;
}

template <class T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T get([[maybe_unused]] ThrowPolicy policy, std::string_view key,
             std::optional<T> default_value = std::nullopt) {
    auto value = get(key, std::string_view{});

    if (value.empty()) {
        return default_value.value();
    }

    T result{};
    auto [ptr, ec] =
        std::from_chars(value.data(), value.data() + value.size(), result);

    // Check for parsing errors or out-of-range values
    if (ec != std::errc{}) {
        errcToException(ec, value);
    }

    return result;
}

bool has(std::string_view key);
void set(std::string_view key, std::string_view value, bool replace = true);
void save(std::string_view path);
} // namespace dotenv
