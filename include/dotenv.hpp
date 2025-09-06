#pragma once

#include <charconv>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

// Include SIMD header if available
#ifdef DOTENV_SIMD_ENABLED
#include "dotenv_simd.hpp"
#endif

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

/**
 * @brief Force traditional implementation (no SIMD, for benchmarking)
 * @param path Path to .env file
 * @param replace Whether to replace existing variables
 * @return Number of variables loaded, or -1 on error
 */
int load_traditional(std::string_view path = ".env", int replace = 1) noexcept;

#ifdef DOTENV_SIMD_ENABLED
/**
 * @brief Load .env file with SIMD optimization when available
 * @param path Path to .env file
 * @param replace Whether to replace existing variables
 * @param force_simd Force SIMD even if auto-detection suggests otherwise
 * @return Number of variables loaded, or -1 on error
 */
int load_simd(std::string_view path = ".env", int replace = 1) noexcept;
#endif
std::string_view get(std::string_view key, std::string_view default_value = "");
std::string get_string(std::string_view key,
                       std::string_view default_value = "");

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
void unset(std::string_view key);
void save(std::string_view path);

// APIs modernas com std::optional
template <class T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline std::optional<T> get_optional(std::string_view key) {
    auto value = get(key, std::string_view{});

    if (value.empty()) {
        return std::nullopt;
    }

    T result{};
    auto [ptr, ec] =
        std::from_chars(value.data(), value.data() + value.size(), result);

    if (ec != std::errc{}) {
        return std::nullopt;
    }

    return result;
}

std::optional<std::string> get_optional_string(std::string_view key);

} // namespace dotenv
