#include "dotenv_mmap.hpp"
#include <filesystem>
#include <stdexcept>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

// RAII wrapper for Unix file descriptor
namespace {
class file_descriptor {
  public:
    explicit file_descriptor(std::string_view filename) noexcept
        : fd_(::open(filename.data(), O_RDONLY, 0)) {}

    ~file_descriptor() noexcept {
        if (fd_ != -1) {
            ::close(fd_);
        }
    }

    file_descriptor(const file_descriptor &) = delete;
    file_descriptor &operator=(const file_descriptor &) = delete;

    file_descriptor(file_descriptor &&other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }

    file_descriptor &operator=(file_descriptor &&other) noexcept {
        if (this != &other) {
            if (fd_ != -1) {
                ::close(fd_);
            }
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    [[nodiscard]] bool valid() const noexcept { return fd_ != -1; }
    [[nodiscard]] int get() const noexcept { return fd_; }
    [[nodiscard]] int release() noexcept {
        int tmp = fd_;
        fd_ = -1;
        return tmp;
    }

  private:
    int fd_ = -1;
};
} // namespace
#endif

namespace dotenv {

mapped_file::mapped_file(std::string_view filename) {
    if (!map(filename)) {
        std::string filename_str{filename}; // Convert for error message
        throw std::runtime_error("Failed to map file: " + filename_str);
    }
}

mapped_file::mapped_file(mapped_file &&other) noexcept
#ifdef _WIN32
    : file_handle_(other.file_handle_), mapping_handle_(other.mapping_handle_),
#else
    : fd_(other.fd_),
#endif
      size_(other.size_), data_(other.data_) {

#ifdef _WIN32
    other.file_handle_ = INVALID_HANDLE_VALUE;
    other.mapping_handle_ = nullptr;
#else
    other.fd_ = -1;
#endif
    other.size_ = 0;
    other.data_ = nullptr;
}

mapped_file &mapped_file::operator=(mapped_file &&other) noexcept {
    if (this == &other) {
        return *this;
    }

    close_impl();

#ifdef _WIN32
    file_handle_ = other.file_handle_;
    mapping_handle_ = other.mapping_handle_;
    other.file_handle_ = INVALID_HANDLE_VALUE;
    other.mapping_handle_ = nullptr;
#else
    fd_ = other.fd_;
    other.fd_ = -1;
#endif

    size_ = other.size_;
    data_ = other.data_;

    other.size_ = 0;
    other.data_ = nullptr;

    return *this;
}

mapped_file::~mapped_file() noexcept { close_impl(); }

bool mapped_file::map(std::string_view filename) {
    close_impl();

#ifdef _WIN32
    // Windows implementation using CreateFileMapping
    file_handle_ =
        CreateFileA(filename.data(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (file_handle_ == INVALID_HANDLE_VALUE) {
        return false;
    }

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file_handle_, &file_size)) {
        CloseHandle(file_handle_);
        file_handle_ = INVALID_HANDLE_VALUE;
        return false;
    }

    if (file_size.QuadPart > SIZE_MAX) {
        CloseHandle(file_handle_);
        file_handle_ = INVALID_HANDLE_VALUE;
        return false;
    }

    size_ = static_cast<size_t>(file_size.QuadPart);

    if (size_ == 0) {
        // Empty file - valid but nothing to map
        data_ = nullptr;
        return true;
    }

    mapping_handle_ =
        CreateFileMappingA(file_handle_, nullptr, PAGE_READONLY, 0, 0, nullptr);

    if (mapping_handle_ == nullptr) {
        CloseHandle(file_handle_);
        file_handle_ = INVALID_HANDLE_VALUE;
        return false;
    }

    data_ = MapViewOfFile(mapping_handle_, FILE_MAP_READ, 0, 0, 0);

    if (data_ == nullptr) {
        CloseHandle(mapping_handle_);
        CloseHandle(file_handle_);
        mapping_handle_ = nullptr;
        file_handle_ = INVALID_HANDLE_VALUE;
        return false;
    }

#else
    // Linux/Unix implementation using mmap with RAII wrapper
    file_descriptor file_desc(filename);
    if (!file_desc.valid()) {
        return false;
    }

    struct stat filest {};
    if (::fstat(file_desc.get(), &filest) == -1) {
        return false;
    }

    if (filest.st_size < 0) {
        return false;
    }

    size_ = static_cast<size_t>(filest.st_size);

    if (size_ == 0) {
        // Empty file - valid but nothing to map
        data_ = nullptr;
        fd_ = file_desc.release(); // Take ownership
        return true;
    }

    data_ = ::mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, file_desc.get(), 0);
    if (data_ == MAP_FAILED) {
        data_ = nullptr;
        return false;
    }

    fd_ = file_desc.release(); // Take ownership
#endif

    return true;
}

void mapped_file::close() noexcept { close_impl(); }

void mapped_file::close_impl() noexcept {
#ifdef _WIN32
    if (data_ != nullptr) {
        UnmapViewOfFile(data_);
        data_ = nullptr;
    }
    if (mapping_handle_ != nullptr) {
        CloseHandle(mapping_handle_);
        mapping_handle_ = nullptr;
    }
    if (file_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(file_handle_);
        file_handle_ = INVALID_HANDLE_VALUE;
    }
#else
    if (data_ != nullptr && data_ != MAP_FAILED) {
        ::munmap(data_, size_);
        data_ = nullptr;
    }
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
#endif
    size_ = 0;
}

} // namespace dotenv
