#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace dotenv {

/**
 * @brief Cross-platform memory-mapped file implementation with RAII
 *
 * Provides zero-copy file access for high-performance .env file parsing.
 * Automatically handles platform differences between Linux/Unix (mmap)
 * and Windows (CreateFileMapping).
 */
class mapped_file {
  public:
    /**
     * @brief Construct and map a file
     * @param filename Path to file to map
     * @throws std::runtime_error if file cannot be opened or mapped
     */
    explicit mapped_file(std::string_view filename);

    /**
     * @brief Default constructor - creates empty mapping
     */
    mapped_file() = default;

    // Delete copy operations
    mapped_file(const mapped_file &) = delete;
    mapped_file &operator=(const mapped_file &) = delete;

    // Move operations
    mapped_file(mapped_file &&other) noexcept;
    mapped_file &operator=(mapped_file &&other) noexcept;

    /**
     * @brief Destructor - automatically unmaps file
     */
    ~mapped_file() noexcept;

    /**
     * @brief Map a new file (closes previous mapping if any)
     * @param filename Path to file to map
     * @return true if mapping successful
     * @throws std::runtime_error if file cannot be opened or mapped
     */
    bool map(std::string_view filename);

    /**
     * @brief Get file size in bytes
     * @return Size of mapped file
     */
    [[nodiscard]] size_t size() const noexcept { return size_; }

    /**
     * @brief Get raw pointer to mapped data
     * @return Pointer to file data, or nullptr if not mapped
     */
    [[nodiscard]] const void *data() const noexcept { return data_; }

    /**
     * @brief Get file content as string_view (zero-copy)
     * @return string_view of entire file content
     */
    [[nodiscard]] std::string_view view() const noexcept {
        if ((data_ != nullptr) && size_ > 0) {
            return std::string_view(static_cast<const char *>(data_), size_);
        }
        return {};
    }

    /**
     * @brief Check if file is currently mapped
     * @return true if file is mapped and accessible
     */
    [[nodiscard]] bool is_mapped() const noexcept {
        return data_ != nullptr && size_ > 0;
    }

    /**
     * @brief Manually unmap the file
     */
    void close() noexcept;

  private:
    void close_impl() noexcept;

#ifdef _WIN32
    HANDLE file_handle_ = INVALID_HANDLE_VALUE;
    HANDLE mapping_handle_ = nullptr;
#else
    int fd_ = -1;
#endif

    size_t size_ = 0;
    void *data_ = nullptr;
};

} // namespace dotenv
