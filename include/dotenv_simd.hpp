#pragma once

#include <cstddef>
#include <functional> // For std::function
#include <optional>
#include <string_view>
#include <unordered_map>

/// SIMD-accelerated dotenv operations using AVX2 when available
namespace dotenv::simd {

/**
 * @brief Check if AVX2 is available at runtime
 * @return true if AVX2 instructions are supported
 */
[[nodiscard]] bool is_avx2_available() noexcept;

/**
 * @brief Count newlines in content using SIMD acceleration
 * @param content Content to scan
 * @param delimiter Line delimiter character
 * @return Number of lines found
 */
[[nodiscard]] auto count_lines_avx2(std::string_view content,
                                    char delimiter = '\n') noexcept -> size_t;

/**
 * @brief Memory-efficient callback-based line processing with SIMD optimization
 *
 * Processes lines one at a time without storing them all in memory.
 * This is inspired by the getLinesCb pattern for maximum memory efficiency.
 *
 * @param content The string view content to process
 * @param delimiter The line delimiter character
 * @param callback Function called for each line: void(line_index, line_view)
 * @return Total number of lines processed
 */
[[nodiscard]] auto process_lines_avx2(
    std::string_view content, char delimiter,
    const std::function<void(size_t, std::string_view)> &callback) noexcept
    -> size_t;

/**
 * @brief High-performance dotenv loading using memory-mapped files and SIMD
 * optimization
 *
 * This function combines zero-copy file access with SIMD line parsing for
 * maximum performance. Uses callback-based processing to minimize memory
 * allocation.
 *
 * @param filename Path to the .env file
 * @return Optional containing environment variables map, or nullopt on error
 */
[[nodiscard]] std::optional<std::unordered_map<std::string, std::string>>
load_simd_mmap(const std::string &filename);

} // namespace dotenv::simd
