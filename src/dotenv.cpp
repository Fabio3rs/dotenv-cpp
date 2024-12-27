#include "dotenv.hpp"
#include "dotenv.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#ifdef _WIN32
#include <Windows.h>
#include <cstdlib>
#define setenv(key, value, overwrite) _putenv_s(key, value)
#define unsetenv(key) _putenv_s(key, "")
#else
#include <unistd.h>
#endif

extern "C" {
int dotenv_load(const char *path, int replace) {
    return dotenv::load(path, replace);
}

const char *dotenv_get(const char *key, const char *default_value) {
    auto value = getenv(key);
    return value ? value : default_value;
}

void dotenv_save(const char *path) { dotenv::save(path); }
}

template <class StrType = std::string>
static auto split_once(std::string_view strview, std::string_view delimiter)
    -> std::pair<StrType, StrType> {
    size_t pos = strview.find(delimiter);
    if (pos == std::string_view::npos) {
        return {StrType(strview), StrType("")}; // Nenhum delimitador encontrado
    }
    return {StrType(strview.substr(0, pos)),
            StrType(strview.substr(pos + delimiter.size()))};
}

int dotenv::load(std::string_view path, int replace) noexcept {
    std::ifstream dotenv((std::filesystem::path(path)));

    if (!dotenv.is_open()) {
        return -1;
    }

    int count = 0;

    std::string line;
    while (std::getline(dotenv, line)) {
        auto [key, value] = split_once(line, "=");

        if (key.empty() || key[0] == '#') {
            continue;
        }

        int res = setenv(key.c_str(), value.c_str(), replace);

        if (res != 0) {
            std::cerr << "Failed to set environment variable: "
                      << std::quoted(key) << std::endl;
        } else {
            count++;
        }
    }

    return count;
}

std::string_view dotenv::get(std::string_view key,
                             std::string_view default_value) {
    auto value = getenv(key.data());
    return value ? value : default_value;
}

bool dotenv::has(std::string_view key) { return getenv(key.data()) != nullptr; }

void dotenv::set(std::string_view key, std::string_view value, bool replace) {
    setenv(key.data(), value.data(), replace ? 1 : 0);
}

void dotenv::save(std::string_view path) {
    std::ofstream dotenv((std::filesystem::path(path)));

    if (!dotenv.is_open()) {
        return;
    }

    char **env = environ;

    while (*env) {
        dotenv << *env << std::endl;
        env++;
    }

    dotenv.close();
}
