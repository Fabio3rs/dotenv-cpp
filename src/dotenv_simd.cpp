#include "dotenv_simd.hpp"

#ifdef DOTENV_SIMD_ENABLED

#include "dotenv_mmap.hpp" // Memory-mapping support
#include <array>
#include <cpuid.h>
#include <cstring>
#include <functional> // For std::function
#include <optional>
#include <unordered_map>

namespace dotenv::simd {

auto is_avx2_available() noexcept -> bool {
    return __builtin_cpu_supports("avx2");
}

auto count_lines_avx2(std::string_view content,
                      char delimiter) noexcept -> size_t {
    if (!is_avx2_available() || content.empty()) {
        return 0;
    }

    const auto line_feed = _mm256_set1_epi8(delimiter);
    size_t line_count = 0;

    for (size_t i = 0; i < content.size(); i += AVX2_VECTOR_SIZE) {
        // Use safe memory copy for type conversion
        const auto remaining = content.size() - i;
        const auto chunk_size =
            (remaining < AVX2_VECTOR_SIZE) ? remaining : AVX2_VECTOR_SIZE;

        __m256i mem;
        if (chunk_size == AVX2_VECTOR_SIZE) {
            std::memcpy(&mem, content.data() + i, AVX2_VECTOR_SIZE);
        } else {
            // Handle partial chunk at end
            alignas(AVX2_ALIGNMENT) std::array<char, AVX2_VECTOR_SIZE> buffer{};
            std::memcpy(buffer.data(), content.data() + i, chunk_size);
            std::memcpy(&mem, buffer.data(), AVX2_VECTOR_SIZE);
        }

        const auto cmp = _mm256_cmpeq_epi8(mem, line_feed);
        const auto mask = static_cast<unsigned int>(_mm256_movemask_epi8(cmp));

        if (mask != 0) {
            line_count += static_cast<size_t>(__builtin_popcount(mask));
        }
    }

    return line_count;
}

// Simplified return type for C++20 compatibility
auto load_simd_mmap(const std::string &filename)
    -> std::optional<std::unordered_map<std::string, std::string>> {
    try {
        // Use memory-mapped file for zero-copy access
        mapped_file mmap_file(filename);

        if (!mmap_file.is_mapped()) {
            return std::nullopt; // Failed to map file
        }

        auto file_view = mmap_file.view();
        if (file_view.empty()) {
            return std::unordered_map<std::string,
                                      std::string>{}; // Empty file is valid
        }

        // Use SIMD-optimized callback-based parsing with zero-copy string_view
        std::unordered_map<std::string, std::string> env_vars;

        // Process lines with callback for memory efficiency
        std::function<void(size_t, std::string_view)> parse_callback =
            [&env_vars](size_t /* line_idx */, std::string_view line) {
                // Skip empty lines and comments
                if (line.empty() || line[0] == '#') {
                    return;
                }

                // Find the '=' separator
                const auto eq_pos = line.find('=');
                if (eq_pos == std::string_view::npos) {
                    return; // Skip malformed lines
                }

                auto key = line.substr(0, eq_pos);
                auto value = line.substr(eq_pos + 1);

                // Trim whitespace efficiently
                while (!key.empty() &&
                       (key.back() == ' ' || key.back() == '\t')) {
                    key.remove_suffix(1);
                }
                while (!key.empty() &&
                       (key.front() == ' ' || key.front() == '\t')) {
                    key.remove_prefix(1);
                }

                // Remove quotes from value if present
                if (value.size() >= 2 &&
                    ((value.front() == '"' && value.back() == '"') ||
                     (value.front() == '\'' && value.back() == '\''))) {
                    value.remove_prefix(1);
                    value.remove_suffix(1);
                }

                if (!key.empty()) {
                    env_vars.emplace(std::string(key), std::string(value));
                }
            };

        [[maybe_unused]] auto line_count =
            process_lines_avx2(file_view, '\n', parse_callback);

        return env_vars;

    } catch (const std::exception &) {
        return std::nullopt; // Error loading file
    }
}

} // namespace dotenv::simd

#endif // DOTENV_SIMD_ENABLED
