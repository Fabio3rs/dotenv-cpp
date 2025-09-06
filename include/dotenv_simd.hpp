#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <functional> // For std::function
#include <immintrin.h>
#include <optional>
#include <string_view>
#include <unordered_map>

/// SIMD-accelerated dotenv operations using AVX2 when available
namespace dotenv::simd {

constexpr inline size_t AVX2_VECTOR_SIZE = 32; // 256 bits / 8 bits per byte
constexpr inline size_t AVX2_ALIGNMENT = 32;

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
template <typename Callback_Type>
[[nodiscard]] auto
process_lines_avx2(std::string_view content, char delimiter,
                   Callback_Type &&callback) noexcept -> size_t {
    if (!is_avx2_available() || content.empty()) {
        return 0;
    }

    const auto line_feed = _mm256_set1_epi8(delimiter);
    const char *current_line = content.data();
    size_t line_count = 0;

    for (size_t i = 0; i < content.size(); i += AVX2_VECTOR_SIZE) {
        // Use safe memory copy approach
        const auto remaining = content.size() - i;
        const auto chunk_size =
            (remaining < AVX2_VECTOR_SIZE) ? remaining : AVX2_VECTOR_SIZE;

        __m256i mem;
        if (chunk_size == AVX2_VECTOR_SIZE) {
            [[likely]];
            mem = _mm256_loadu_si256(
                reinterpret_cast<const __m256i *>(content.data() + i));
        } else {
            alignas(AVX2_ALIGNMENT) std::array<char, AVX2_VECTOR_SIZE> buffer{};
            std::memcpy(buffer.data(), content.data() + i, chunk_size);
            mem = _mm256_loadu_si256(
                reinterpret_cast<const __m256i *>(buffer.data()));
        }

        const auto cmp = _mm256_cmpeq_epi8(mem, line_feed);
        auto mask = static_cast<unsigned int>(_mm256_movemask_epi8(cmp));

        if (mask != 0) {
            const auto found_lines =
                static_cast<unsigned int>(std::popcount(mask));
            size_t relative_pos = i;

            for (unsigned int line_idx = 0; line_idx < found_lines;
                 ++line_idx) {
                const auto pos =
                    static_cast<unsigned int>(std::countr_zero(mask));
                const auto abs_pos = relative_pos + pos;

                // Ensure we don't go beyond the content
                if (abs_pos < content.size()) {
                    const auto *line_end = content.data() + abs_pos;
                    const auto line_size =
                        static_cast<size_t>(line_end - current_line);

                    // Call the callback immediately with the line
                    callback(line_count,
                             std::string_view(current_line, line_size));
                    current_line = content.data() + abs_pos +
                                   1; // Safe pointer calculation
                    ++line_count;
                }

                relative_pos += pos + 1;
                mask >>= pos + 1;
            }
        }
    }

    // Handle remaining content (last line without delimiter)
    const auto remaining_size =
        static_cast<size_t>((content.data() + content.size()) - current_line);
    if (remaining_size > 0) {
        callback(line_count, std::string_view(current_line, remaining_size));
        ++line_count;
    }

    return line_count;
}

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
