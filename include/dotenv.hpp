#pragma once

#include "dotenv_errors.h"
#include "dotenv_types.h"
#include <cerrno>
#include <charconv>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

// C++23 std::expected support (controlled by CMake detection)
#ifndef DOTENV_HAS_EXPECTED
// Fallback detection if CMake didn't set it. Use feature-test macro if
// available and check for the header as a secondary measure.
#if defined(__has_include)
#if __has_include(<expected>) && (defined(__cpp_lib_expected) || (__cplusplus >= 202300L))
#define DOTENV_HAS_EXPECTED 1
#else
#define DOTENV_HAS_EXPECTED 0
#endif
#else
#define DOTENV_HAS_EXPECTED 0
#endif
#endif

// Carefully enable std::expected only when both the build system claims
// support (DOTENV_HAS_EXPECTED) and the header/feature test is actually
// available in this compilation environment. This avoids mismatches when
// CMake detection is optimistic but the standard library doesn't provide
// the expected implementation for this toolchain.
#if defined(DOTENV_HAS_EXPECTED) && (DOTENV_HAS_EXPECTED)
#if defined(__has_include)
#if __has_include(<expected>) && (defined(__cpp_lib_expected) || (__cplusplus >= 202300L))
#include <expected>
#define DOTENV_HAS_STD_EXPECTED 1
#else
/* Header missing or feature macro not present */
#define DOTENV_HAS_STD_EXPECTED 0
#endif
#else
/* Conservative fallback: if DOTENV_HAS_EXPECTED is set but we
   can't test for header presence, assume not available. */
#define DOTENV_HAS_STD_EXPECTED 0
#endif
#else
#define DOTENV_HAS_STD_EXPECTED 0
#endif

// Include SIMD header if available
#ifdef DOTENV_SIMD_ENABLED
#include "dotenv_simd.hpp"
#endif

namespace dotenv {

inline void errc_to_exception(std::errc error_code, std::string_view value) {
    switch (error_code) {
    case std::errc::invalid_argument:
        throw std::invalid_argument("Invalid argument: " + std::string(value));
    case std::errc::result_out_of_range:
        throw std::out_of_range("Result out of range: " + std::string(value));
    default:
        throw std::runtime_error(std::string("Unknown error: ") +
                                 std::to_string(int(error_code)) + " " +
                                 std::string(value));
    }
}

// ==== Process Environment Application ====

/**
 * @brief Apply internally stored variables to process environment
 * @param overwrite_policy Policy for existing variables
 * @note Modern interface for applying loaded variables to process environment
 */
void apply_internal_to_process_env(
    overwrite overwrite_policy = overwrite::replace);

// Legacy compatibility wrapper
[[deprecated("Use apply_internal_to_process_env(overwrite) instead")]]
inline void write_system_env_from_env_map(int replace = 1) {
    apply_internal_to_process_env((replace != 0) ? overwrite::replace
                                                 : overwrite::preserve);
}

// ==== Core C++20 API (Primary Interface) ====
// When C++23 std::expected is available we expose the modern APIs below.
// To avoid functions that differ only by return type, the legacy pair-returning
// overloads are only declared when std::expected is NOT available.
// Legacy C++20 pair-returning APIs (renamed). These provide the older
// std::pair-based interface and are intended for code that hasn't migrated
// to the C++23 std::expected APIs; names end with `_legacy` to avoid
// overloads that differ only by return type.
/**
 * @brief Legacy pair-returning load API (renamed)
 */
std::pair<dotenv_error, int>
load_legacy(std::string_view path = ".env",
            const load_options &options = {}) noexcept;

/**
 * @brief Legacy pair-returning load_traditional API (renamed)
 */
std::pair<dotenv_error, int>
load_traditional_legacy(std::string_view path = ".env",
                        const load_options &options = {}) noexcept;

#ifdef DOTENV_SIMD_ENABLED
/**
 * @brief Legacy pair-returning SIMD API (renamed)
 */
std::pair<dotenv_error, int>
load_simd_legacy(std::string_view path = ".env",
                 const load_options &options = {}) noexcept;
#endif

// Note: modern std::expected-based APIs are declared in the 'Modern Load'
// section below (names: load, load_traditional, load_simd). The legacy
// pair-returning APIs are available as *_legacy names.

/**
 * @brief Apply internal environment variables to the current process
 * environment
 * @param overwrite_policy Variable overwrite policy for existing process
 * variables
 * @note This function applies all internally stored variables to the process
 * environment via setenv()
 * @note Use this when you've loaded variables with apply=process_env_apply::no
 * and want to apply them later
 * @note overwrite::replace maps to setenv(name, val, overwrite=1)
 * @note overwrite::preserve maps to setenv(name, val, overwrite=0)
 */

// ==== Modern Load API (Primary Interface) ====

#if DOTENV_HAS_STD_EXPECTED
/**
 * @brief Load environment variables from file with modern type-safe API
 * (C++23+)
 * @param path Path to the .env file
 * @param options Load configuration with type-safe enum options
 * @return std::expected<int, dotenv_error> where value is variables loaded
 * count, or error
 * @note This is the preferred modern interface with type-safe configuration
 * @note Requires C++23 and std::expected support
 */
std::expected<int, dotenv::dotenv_error>
load(std::string_view path, const load_options &options = {}) noexcept;

/**
 * @brief Modern std::expected-based traditional backend
 */
std::expected<int, dotenv::dotenv_error>
load_traditional(std::string_view path,
                 const load_options &options = {}) noexcept;

#ifdef DOTENV_SIMD_ENABLED
std::expected<int, dotenv::dotenv_error>
load_simd(std::string_view path, const load_options &options = {}) noexcept;
#endif
#endif // DOTENV_HAS_STD_EXPECTED

/**
 * @brief Load environment variables with detailed status information
 * @param path Path to the .env file (default: ".env")
 * @param options Load configuration options
 * @return std::pair<dotenv_error_t, int> where first is status code and second
 * is variables loaded count
 * @note On success, returns {DOTENV_SUCCESS, count}. On error, returns
 * {error_code, 0}
 */
std::pair<dotenv_error_t, int>
load_status(std::string_view path = ".env",
            const load_options &options = {}) noexcept;

// C++23 std::expected API - Modern error handling
// NOTE: modern std::expected-based APIs are declared elsewhere above/below
// under the DOTENV_HAS_EXPECTED guard. Avoid duplicating prototypes here.

// ==== Legacy/Compatibility API ====

/**
 * @brief Load environment variables (legacy int/bool interface)
 * @deprecated Use load(path, load_options{.overwrite_policy=...,
 * .apply_to_process=...}) instead
 */
[[deprecated("Use load(path, load_options{.overwrite_policy=..., "
             ".apply_to_process=...}) instead")]]
int load(std::string_view path, int replace, bool apply_system_env) noexcept;

/**
 * @brief Force traditional implementation (legacy interface)
 * @deprecated Use load_traditional(path, load_options{...}) instead
 */
[[deprecated("Use load_traditional(path, load_options{...}) instead")]]
int load_traditional(std::string_view path, int replace,
                     bool apply_system_env) noexcept;

/**
 * @brief Load with status (legacy interface)
 * @deprecated Use modern load() with std::expected instead
 */
[[deprecated("Use modern load() with std::expected instead")]]
std::pair<dotenv_error_t, int> load_with_status(std::string_view path,
                                                int replace,
                                                bool apply_system_env) noexcept;

/**
 * @brief Load traditional with status (legacy interface)
 * @deprecated Use modern load_traditional() with std::expected instead
 */
[[deprecated("Use modern load_traditional() with std::expected instead")]]
std::pair<dotenv_error_t, int>
load_traditional_with_status(std::string_view path, int replace,
                             bool apply_system_env) noexcept;

/**
 * @brief Set variable (legacy bool interface)
 * @deprecated Use set(key, value, overwrite) instead
 */
[[deprecated("Use set(key, value, overwrite) instead")]]
void set(std::string_view key, std::string_view value, bool replace);

/**
 * @brief Force traditional implementation (no SIMD, for benchmarking)
 * @param path Path to .env file
 * @param options Load configuration options (backend will be forced to
 * traditional)
 * @return Number of variables loaded, or -1 on error
 * @note This function bypasses SIMD auto-detection and uses traditional file
 * parsing
 */
// Note: SIMD-specific prototypes are declared above when DOTENV_SIMD_ENABLED is
// set and when DOTENV_HAS_EXPECTED is true (inside the Modern Load API guard).
// No additional prototypes are required here.

// ==== Variable Access API (Consistent Naming) ====

/**
 * @brief Get environment variable value with fallback
 * @param key Variable name to retrieve
 * @param default_value Value to return if key not found
 * @return Variable value as string_view or default_value
 * @note Primary getter function - returns string_view for efficiency
 */
std::string_view get(std::string_view key, std::string_view default_value = "");

/**
 * @brief Get environment variable value as owned string
 * @param key Variable name to retrieve
 * @param default_value Default value if variable not found
 * @return Variable value as std::string or default_value
 * @note Returns owned string copy - use get() for string_view efficiency
 */
std::string value(std::string_view key, std::string_view default_value = "");

/**
 * @brief Get environment variable with fallback (never throws)
 * @param key Variable name to retrieve
 * @param fallback_value Default value if variable not found
 * @return Variable value as std::string or fallback_value
 * @note Consistent with numeric value_or<T>() family
 */
std::string value_or(std::string_view key, std::string_view fallback_value);

/**
 * @brief Try to get environment variable value
 * @param key Variable name to retrieve
 * @return std::optional<std::string> containing value or std::nullopt if not
 * found
 * @note Optional-based access for conditional configuration
 */
std::optional<std::string> try_value(std::string_view key) noexcept;

/**
 * @brief Check if environment variable exists
 * @param key Variable name to check
 * @return True if variable exists, false otherwise
 * @note Preferred name following standard library conventions
 */
bool contains(std::string_view key);

/**
 * @brief Set environment variable with type-safe overwrite policy
 * @param key Variable name
 * @param value Variable value
 * @param overwrite_policy Policy for existing variables (maps to setenv
 * overwrite parameter)
 */
void set(std::string_view key, std::string_view value,
         overwrite overwrite_policy = overwrite::replace);

/**
 * @brief Remove environment variable
 * @param key Variable name to remove
 */
void unset(std::string_view key);

/**
 * @brief Save current environment variables to file
 * @param path Path to save the .env file
 * @note Only saves internally managed variables (loaded from .env files) for
 * security
 */
void save_to_file(std::string_view path);

// ==== Numeric Type Conversion Templates ====

// Helper: parse arithmetic values from string_view
// Uses std::from_chars for integral types and the C library for floating
// point types as libc++ may not provide from_chars for floating values.
template <class T>
inline bool parse_arithmetic_from_string(std::string_view str,
                                         T &out) noexcept {
    if (str.empty()) {
        return false;
    }

    if constexpr (std::is_integral_v<T>) {
        auto [ptr, ec] =
            std::from_chars(str.data(), str.data() + str.size(), out);
        (void)ptr;
        return (ec == std::errc{});
    } else if constexpr (std::is_floating_point_v<T>) {
        // Use a temporary null-terminated buffer for the C functions.
        std::string tmp(str);
        const char *cstr = tmp.c_str();
        char *endptr = nullptr;
        errno = 0;
        if constexpr (std::is_same_v<T, float>) {
            out = std::strtof(cstr, &endptr);
        } else if constexpr (std::is_same_v<T, double>) {
            out = std::strtod(cstr, &endptr);
        } else {
            out = std::strtold(cstr, &endptr);
        }
        const size_t parsed_len = static_cast<size_t>(endptr - cstr);
        if (parsed_len != tmp.size()) {
            return false; // trailing garbage or partial parse
        }
        if (errno == ERANGE) {
            return false; // out of range
        }
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Get environment variable value with fallback (never throws)
 * @param key Variable name to retrieve
 * @param fallback_value Default value if variable not found or conversion fails
 * @return Variable value as T or fallback_value
 * @note This function provides noexcept guarantee and never throws exceptions
 * @note Consistent with string value_or() function
 */
template <class T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T value_or(std::string_view key, T fallback_value) noexcept {
    auto value_str = get(key, std::string_view{});

    if (value_str.empty()) {
        return fallback_value;
    }

    T result{};
    if (!parse_arithmetic_from_string<T>(value_str, result)) {
        return fallback_value;
    }

    return result;
}

/**
 * @brief Get required environment variable value (throws on missing/invalid)
 * @param key Variable name to retrieve
 * @return Variable value as T
 * @throws std::invalid_argument if variable not found
 * @throws std::out_of_range if value cannot be converted to T
 * @note Use this when the variable is mandatory for application logic
 */
template <class T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T value_required(std::string_view key) {
    auto value_str = get(key, std::string_view{});

    if (value_str.empty()) {
        throw std::invalid_argument(
            "Required environment variable not found: " + std::string(key));
    }

    T result{};
    if (!parse_arithmetic_from_string<T>(value_str, result)) {
        // Map parsing failures to appropriate exceptions
        throw std::invalid_argument("Invalid numeric format: " +
                                    std::string(value_str));
    }

    return result;
}

/**
 * @brief Try to get environment variable value (optional-based)
 * @param key Variable name to retrieve
 * @return std::optional<T> containing value or std::nullopt if not
 * found/invalid
 * @note This function provides std::optional semantics for optional
 * configuration
 */
template <class T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline std::optional<T> try_value(std::string_view key) noexcept {
    auto value_str = get(key, std::string_view{});

    if (value_str.empty()) {
        return std::nullopt;
    }

    T result{};
    if (!parse_arithmetic_from_string<T>(value_str, result)) {
        return std::nullopt;
    }

    return result;
}

// ==== C++23 Enhanced Get API ====

#if DOTENV_HAS_STD_EXPECTED
/**
 * @brief Get required environment variable value with std::expected (C++23+)
 * @param key Variable name to retrieve
 * @return std::expected<std::string, dotenv_error> containing value or error
 * @note Enhanced C++23 API with modern error handling
 */
std::expected<std::string, dotenv_error> value_expected(std::string_view key);

/**
 * @brief Get required numeric value with std::expected (C++23+)
 * @param key Variable name to retrieve
 * @return std::expected<T, dotenv_error> containing value or error
 * @note Enhanced C++23 API with type-safe error handling
 */
template <class T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline std::expected<T, dotenv_error> value_expected(std::string_view key) {
    auto value_str = get(key, std::string_view{});

    if (value_str.empty()) {
        return std::unexpected(dotenv_error::key_not_found);
    }

    T result{};
    auto [ptr, error_code] = std::from_chars(
        value_str.data(), value_str.data() + value_str.size(), result);

    if (error_code == std::errc::invalid_argument) {
        return std::unexpected(dotenv_error::invalid_format);
    }
    if (error_code == std::errc::result_out_of_range) {
        return std::unexpected(dotenv_error::invalid_argument);
    }
    if (error_code != std::errc{}) {
        return std::unexpected(dotenv_error::invalid_format);
    }

    return result;
}
#endif // DOTENV_HAS_STD_EXPECTED

// ==== Legacy API Compatibility (Minimal Deprecated Aliases) ====

/**
 * @deprecated Use contains() instead - follows standard library naming
 */
[[deprecated("Use contains() instead - follows standard library naming")]]
inline bool has(std::string_view key) {
    return contains(key);
}

/**
 * @deprecated Use value_or<T>() instead for consistent naming
 */
template <class T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
[[deprecated("Use value_or<T>() instead for consistent naming")]]
inline T get_or(std::string_view key, T fallback_value) noexcept {
    return value_or(key, fallback_value);
}

/**
 * @deprecated Use value_required<T>() instead for consistent naming
 */
template <class T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
[[deprecated("Use value_required<T>() instead for consistent naming")]]
inline T get_required(std::string_view key) {
    return value_required<T>(key);
}

/**
 * @deprecated Use try_value<T>() instead for consistent naming
 */
template <class T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
[[deprecated("Use try_value<T>() instead for consistent naming")]]
inline std::optional<T> try_get(std::string_view key) noexcept {
    return try_value<T>(key);
}

/**
 * @deprecated Use save_to_file() for explicit destination clarity
 */
[[deprecated("Use save_to_file() for explicit destination clarity")]]
inline void save(std::string_view path) {
    save_to_file(path);
}

/**
 * @deprecated Use set() with overwrite enum for type safety
 */
[[deprecated("Use set() with overwrite enum for type safety")]]
inline void set(std::string_view key, std::string_view value, bool replace) {
    set(key, value, replace ? overwrite::replace : overwrite::preserve);
}

} // namespace dotenv
