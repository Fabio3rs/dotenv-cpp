#pragma once

#include <string_view>

namespace dotenv {
int load(std::string_view path = ".env", int replace = 1) noexcept;
std::string_view get(std::string_view key, std::string_view default_value = "");
bool has(std::string_view key);
void set(std::string_view key, std::string_view value, bool replace = true);
void save(std::string_view path);
} // namespace dotenv
