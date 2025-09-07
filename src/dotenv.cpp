#include "dotenv.hpp"
#include "dotenv.h"
#include "dotenv_types.h"
#include <algorithm>
#include <cctype>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#if DOTENV_HAS_STD_EXPECTED
#include <expected>
#endif
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#ifdef DOTENV_SIMD_ENABLED
#include "dotenv_mmap.hpp"
#include "dotenv_simd.hpp"
#endif

// Forward declarations for internal/renamed functions to satisfy out-of-line
// definitions and internal callsites.
namespace dotenv {
auto load_raw(std::string_view path, int replace,
              bool apply_system_env) noexcept -> int;
auto load_traditional_raw(std::string_view path, int replace,
                          bool apply_system_env) noexcept -> int;
std::pair<dotenv::dotenv_error, int>
load_legacy(std::string_view path, const load_options &options) noexcept;
std::pair<dotenv::dotenv_error, int>
load_traditional_legacy(std::string_view path,
                        const load_options &options) noexcept;
std::pair<dotenv::dotenv_error, int>
load_simd_legacy(std::string_view path, const load_options &options) noexcept;
} // namespace dotenv

#ifdef _WIN32
#include <Windows.h>
#include <codecvt>
#include <cstdlib>
#include <locale>

// Conversão UTF-16 para UTF-8 no Windows
static std::string utf16_to_utf8(const std::wstring &utf16_str) {
    if (utf16_str.empty())
        return {};

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(),
                                          static_cast<int>(utf16_str.size()),
                                          nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0)
        return {};

    std::string utf8_str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(),
                        static_cast<int>(utf16_str.size()), utf8_str.data(),
                        size_needed, nullptr, nullptr);
    return utf8_str;
}

// Conversão UTF-8 para UTF-16 no Windows
static std::wstring utf8_to_utf16(const std::string &utf8_str) {
    if (utf8_str.empty())
        return {};

    int size_needed =
        MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(),
                            static_cast<int>(utf8_str.size()), nullptr, 0);
    if (size_needed <= 0)
        return {};

    std::wstring utf16_str(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(),
                        static_cast<int>(utf8_str.size()), utf16_str.data(),
                        size_needed);
    return utf16_str;
}

// Função utilitária portável que respeita o parâmetro replace
static int set_env(const char *key, const char *value, int replace) {
    if (!replace) {
        // Verifica se a variável já existe usando Unicode
        std::wstring wide_key = utf8_to_utf16(key);
        wchar_t *existing_value = nullptr;
        size_t len = 0;
        errno_t err = _wdupenv_s(&existing_value, &len, wide_key.c_str());
        if (err == 0 && existing_value != nullptr) {
            free(existing_value);
            return 0; // Não substitui se já existe
        }
        if (existing_value) {
            free(existing_value);
        }
    }

    // Usar Unicode para definir variável
    std::wstring wide_key = utf8_to_utf16(key);
    std::wstring wide_value = utf8_to_utf16(value);
    return _wputenv_s(wide_key.c_str(), wide_value.c_str());
}

// Função para carregar ambiente do Windows usando Unicode
/*
static void load_windows_environment(std::unordered_map<std::string,
std::string>& env_map) { wchar_t* env_strings = GetEnvironmentStringsW(); if
(!env_strings) return;

    wchar_t* current = env_strings;
    while (*current) {
        std::wstring env_line(current);

        // Pular para próxima string
        current += env_line.length() + 1;

        // Ignorar entradas que começam com '=' (são especiais do Windows)
        if (env_line.empty() || env_line[0] == L'=') continue;

        // Encontrar separador '='
        size_t eq_pos = env_line.find(L'=');
        if (eq_pos == std::wstring::npos) continue;

        // Extrair chave e valor
        std::wstring key_wide = env_line.substr(0, eq_pos);
        std::wstring value_wide = env_line.substr(eq_pos + 1);

        // Converter para UTF-8
        std::string key = utf16_to_utf8(key_wide);
        std::string value = utf16_to_utf8(value_wide);

        if (!key.empty()) {
            env_map[key] = value;
        }
    }

    FreeEnvironmentStringsW(env_strings);
}
*/

#else
#include <unistd.h>
static auto set_env(const char *key, const char *value, int replace) -> int {
    return setenv(key, value, replace);
}

// Função para carregar ambiente POSIX
/*
static void load_posix_environment(std::unordered_map<std::string, std::string>&
env_map) { for (char **env = environ; *env; env++) { std::string env_line(*env);
        size_t eq_pos = env_line.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string key = env_line.substr(0, eq_pos);
        std::string value = env_line.substr(eq_pos + 1);

        if (!key.empty()) {
            env_map[key] = value;
        }
    }
}
*/
#endif

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

// Função para remover espaços em branco no início e fim
static auto trim(std::string_view str) -> std::string_view {
    const auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string_view::npos) {
        return {};
    }
    const auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// Função para processar aspas e escape básico
static auto process_quoted_value(std::string_view value) -> std::string {
    auto trimmed = trim(value);

    // Se não tem aspas, retorna como está
    if (trimmed.size() < 2) {
        return std::string(trimmed);
    }

    // Verifica se está entre aspas simples ou duplas
    char quote_char = '\0';
    if ((trimmed.front() == '"' && trimmed.back() == '"') ||
        (trimmed.front() == '\'' && trimmed.back() == '\'')) {
        quote_char = trimmed.front();
        trimmed = trimmed.substr(1, trimmed.size() - 2); // Remove aspas
    }

    // Se não tinha aspas, retorna trimmed
    if (quote_char == '\0') {
        return std::string(trimmed);
    }

    // Processa escape básico apenas para aspas duplas
    std::string result;
    result.reserve(trimmed.size());

    for (size_t i = 0; i < trimmed.size(); ++i) {
        if (quote_char == '"' && trimmed[i] == '\\' && i + 1 < trimmed.size()) {
            char next = trimmed[i + 1];
            switch (next) {
            case 'n':
                result += '\n';
                break;
            case 't':
                result += '\t';
                break;
            case 'r':
                result += '\r';
                break;
            case '\\':
                result += '\\';
                break;
            case '"':
                result += '"';
                break;
            default:
                result += '\\';
                result += next;
                break;
            }
            ++i; // Pula o próximo caractere
        } else {
            result += trimmed[i];
        }
    }

    return result;
}

// Limites de segurança para evitar DoS
static constexpr size_t MAX_LINE_LENGTH = 8192;
static constexpr size_t MAX_KEY_LENGTH = 256;
static constexpr size_t MAX_VALUE_LENGTH = 4096;

// Threshold baseado em dados empíricos: SIMD só vale a pena para arquivos >50KB
static constexpr size_t MIN_FILE_SIZE_FOR_SIMD =
    static_cast<const size_t>(50U * 1024U);

// Validação de chave (deve ser um identificador válido)
static auto is_valid_key(std::string_view key) -> bool {
    if (key.empty() || key.size() > MAX_KEY_LENGTH) {
        return false;
    }

    // Primeira char deve ser letra ou underscore
    if ((std::isalpha(key[0]) == 0) && key[0] != '_') {
        return false;
    }

    // Restante deve ser alfanumérico ou underscore
    for (size_t i = 1; i < key.size(); ++i) {
        if ((std::isalnum(key[i]) == 0) && key[i] != '_') {
            return false;
        }
    }

    return true;
}

// Declaração antecipada da implementação tradicional
static auto
load_traditional_implementation(std::string_view path, int replace,
                                bool apply_system_env = true) noexcept -> int;

namespace {

// Estrutura para armazenar valor e flag de gerenciamento
struct ValueStruct {
    std::string data;
    bool managedKey{};

    ValueStruct() = default;
    ValueStruct(std::string value, bool managed)
        : data(std::move(value)), managedKey(managed) {}
};

// Mapa seguro que possui a memória das strings e sincronização para threads
std::unordered_map<std::string, ValueStruct> envMap;
std::mutex envMapMutex;

inline void processLine(std::string_view line, [[maybe_unused]] int replace,
                        int &count) {
    auto trimmed_line = trim(line);

    // Early returns para filtros
    if (trimmed_line.empty() || trimmed_line[0] == '#') {
        return;
    }

    if (trimmed_line.size() > MAX_LINE_LENGTH) {
        return;
    }

    // Buscar primeiro '=' - parsing simplificado e correto
    // Aspas só importam para o VALOR, não durante a busca do '='
    size_t eq_pos = trimmed_line.find('=');

    // Early return: sem separador '='
    if (eq_pos == std::string_view::npos) {
        return;
    }

    auto raw_key = trim(trimmed_line.substr(0, eq_pos));

    // Early return: chave inválida
    if (!is_valid_key(raw_key)) {
        return;
    }

    auto raw_value = trimmed_line.substr(eq_pos + 1);
    auto processed_value = process_quoted_value(raw_value);

    // Early return: valor muito longo
    if (processed_value.size() > MAX_VALUE_LENGTH) {
        processed_value.resize(MAX_VALUE_LENGTH);
    }

    std::lock_guard<std::mutex> lock(envMapMutex);
    std::string key_str(raw_key);
    count++;
    if (replace != 0) {
        envMap.insert_or_assign(std::move(key_str),
                                ValueStruct(std::move(processed_value), true));
    } else {
        // Se replace=false, só insere se não existir
        envMap.emplace(std::move(key_str),
                       ValueStruct(std::move(processed_value), true));
    }
}

} // namespace

extern "C" {
/* Helper function to parse boolean values */
static auto parse_bool(const char *value, int default_value) -> int {
    if (value == nullptr) {
        return default_value;
    }

    std::string lower_value = value;
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (lower_value == "true" || lower_value == "1" || lower_value == "yes" ||
        lower_value == "on") {
        return 1;
    }
    if (lower_value == "false" || lower_value == "0" || lower_value == "no" ||
        lower_value == "off") {
        return 0;
    }

    return default_value;
}

/* Core loading functions */
auto dotenv_load(const char *path, int replace, int apply_system_env) -> int {
    const char *file_path = (path != nullptr) ? path : ".env";
    return dotenv::load_raw(file_path, replace, apply_system_env != 0);
}

auto dotenv_load_ex(const char *path, const dotenv_load_options_t *options,
                    dotenv_load_stats_t *stats) -> dotenv_error_t {
    if (path == nullptr) {
        path = ".env";
    }

    // Initialize stats if provided
    if (stats != nullptr) {
        memset(stats, 0, sizeof(*stats));
    }

    // Use default options if not provided
    dotenv_load_options_t default_opts;
    if (options == nullptr) {
        dotenv_get_default_options(&default_opts);
        options = &default_opts;
    }

    try {
        int result = dotenv::load_raw(path, options->replace_existing,
                                      options->apply_to_system != 0);

        if (result < 0) {
            return static_cast<dotenv_error_t>(result);
        }

        if (stats != nullptr) {
            stats->variables_loaded = result;
            // Additional stats would need implementation in the core library
        }

        return DOTENV_SUCCESS;
    } catch (const std::exception &) {
        return DOTENV_ERROR_INVALID_FORMAT;
    }
}

auto dotenv_load_traditional(const char *path, int replace,
                             int apply_system_env) -> int {
    const char *file_path = (path != nullptr) ? path : ".env";
    return dotenv::load_traditional_raw(file_path, replace,
                                        apply_system_env != 0);
}

/* Variable access functions */
auto dotenv_get(const char *key, const char *default_value) -> const char * {
    if (key == nullptr) {
        return (default_value != nullptr) ? default_value : "";
    }

    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    auto it = envMap.find(key_str);

    if (it != envMap.end()) {
        return it->second.data.c_str();
    }

    auto *value = getenv(key);
    return (value != nullptr)
               ? value
               : ((default_value != nullptr) ? default_value : "");
}

auto dotenv_get_buffer(const char *key, char *buffer, size_t buffer_size)
    -> dotenv_error_t {
    if ((key == nullptr) || (buffer == nullptr) || buffer_size == 0) {
        return DOTENV_ERROR_INVALID_ARGUMENT;
    }

    const char *value = dotenv_get(key, nullptr);
    if (value == nullptr) {
        if (buffer_size > 0) {
            buffer[0] = '\0';
        }
        return DOTENV_SUCCESS;
    }

    size_t value_len = strlen(value);
    if (value_len >= buffer_size) {
        return DOTENV_ERROR_BUFFER_TOO_SMALL;
    }

    strcpy(buffer, value);
    return DOTENV_SUCCESS;
}

auto dotenv_has(const char *key) -> int {
    if (key == nullptr) {
        return 0;
    }
    return dotenv::contains(key) ? 1 : 0;
}

auto dotenv_get_int(const char *key, int default_value) -> int {
    const char *value = dotenv_get(key, nullptr);
    if ((value == nullptr) || *value == '\0') {
        return default_value;
    }

    char *endptr = nullptr;
    long result = strtol(value, &endptr, 10);

    // Check for conversion errors or overflow
    if (*endptr != '\0' || result < INT_MIN || result > INT_MAX) {
        return default_value;
    }

    return static_cast<int>(result);
}

auto dotenv_get_long(const char *key, long default_value) -> long {
    const char *value = dotenv_get(key, nullptr);
    if ((value == nullptr) || *value == '\0') {
        return default_value;
    }

    char *endptr = nullptr;
    long result = strtol(value, &endptr, 10);

    if (*endptr != '\0') {
        return default_value;
    }

    return result;
}

auto dotenv_get_double(const char *key, double default_value) -> double {
    const char *value = dotenv_get(key, nullptr);
    if ((value == nullptr) || *value == '\0') {
        return default_value;
    }

    char *endptr = nullptr;
    double result = strtod(value, &endptr);

    if (*endptr != '\0') {
        return default_value;
    }

    return result;
}

auto dotenv_get_bool(const char *key, int default_value) -> int {
    const char *value = dotenv_get(key, nullptr);
    return parse_bool(value, default_value);
}

/* Variable modification functions */
auto dotenv_set(const char *key, const char *value, int replace)
    -> dotenv_error_t {
    if ((key == nullptr) || (value == nullptr)) {
        return DOTENV_ERROR_INVALID_ARGUMENT;
    }

    try {
        dotenv::set(key, value,
                    (replace != 0) ? dotenv::overwrite::replace
                                   : dotenv::overwrite::preserve);
        return DOTENV_SUCCESS;
    } catch (const std::exception &) {
        return DOTENV_ERROR_INVALID_ARGUMENT;
    }
}

auto dotenv_unset(const char *key) -> dotenv_error_t {
    if (key == nullptr) {
        return DOTENV_ERROR_INVALID_ARGUMENT;
    }

    try {
        dotenv::unset(key);
        return DOTENV_SUCCESS;
    } catch (const std::exception &) {
        return DOTENV_ERROR_INVALID_ARGUMENT;
    }
}

/* File operations */
auto dotenv_save(const char *path) -> dotenv_error_t {
    const char *file_path = (path != nullptr) ? path : ".env";

    try {
        dotenv::save(file_path);
        return DOTENV_SUCCESS;
    } catch (const std::exception &) {
        return DOTENV_ERROR_PERMISSION_DENIED;
    }
}

/* Utility functions */
void dotenv_get_default_options(dotenv_load_options_t *options) {
    if (options == nullptr) {
        return;
    }

    options->replace_existing = 1;
    options->apply_to_system = 1;
    options->max_line_length = 0;  // Use library default
    options->max_key_length = 0;   // Use library default
    options->max_value_length = 0; // Use library default
}

auto dotenv_get_error_message(dotenv_error_t error_code) -> const char * {
    switch (error_code) {
    case DOTENV_SUCCESS:
        return "Success";
    case DOTENV_ERROR_FILE_NOT_FOUND:
        return "File not found";
    case DOTENV_ERROR_PERMISSION_DENIED:
        return "Permission denied";
    case DOTENV_ERROR_INVALID_FORMAT:
        return "Invalid file format";
    case DOTENV_ERROR_OUT_OF_MEMORY:
        return "Out of memory";
    case DOTENV_ERROR_INVALID_ARGUMENT:
        return "Invalid argument";
    case DOTENV_ERROR_BUFFER_TOO_SMALL:
        return "Buffer too small";
    default:
        return "Unknown error";
    }
}

auto dotenv_get_version(int *major, int *minor, int *patch) -> const char * {
    static const char *version = "2.0.0";

    if (major != nullptr) {
        *major = 2;
    }
    if (minor != nullptr) {
        *minor = 0;
    }
    if (patch != nullptr) {
        *patch = 0;
    }

    return version;
}

/* Advanced functions */
auto dotenv_enumerate(dotenv_iterator_t iterator, void *user_data) -> int {
    if (iterator == nullptr) {
        return DOTENV_ERROR_INVALID_ARGUMENT;
    }

    std::lock_guard<std::mutex> lock(envMapMutex);

    int count = 0;
    for (const auto &[key, value] : envMap) {
        if (iterator(key.c_str(), value.data.c_str(), user_data) != 0) {
            break; // Iterator requested stop
        }
        count++;
    }

    return count;
}

auto dotenv_clear(int clear_system) -> dotenv_error_t {
    std::lock_guard<std::mutex> lock(envMapMutex);

    if (clear_system != 0) {
        // Clear from system environment
        for (const auto &[key, value] : envMap) {
            if (value.managedKey) {
#ifdef _WIN32
                std::string env_str = key + "=";
                _putenv(env_str.c_str());
#else
                unsetenv(key.c_str());
#endif
            }
        }
    }

    // Clear internal storage
    envMap.clear();

    return DOTENV_SUCCESS;
}
}

void dotenv::apply_internal_to_process_env(overwrite overwrite_policy) {
    std::lock_guard<std::mutex> lock(envMapMutex);
    int replace_flag = (overwrite_policy == overwrite::replace) ? 1 : 0;
    for (const auto &pair : envMap) {
        set_env(pair.first.c_str(), pair.second.data.c_str(), replace_flag);
    }
}

auto dotenv::load_raw(std::string_view path, int replace,
                      bool apply_system_env) noexcept -> int {
#ifdef DOTENV_SIMD_ENABLED
    // Auto-detecção inteligente: usar SIMD sempre que disponível

    // Early return: verificar se AVX2 está disponível
    if (!simd::is_avx2_available()) {
        return load_traditional_implementation(path, replace, apply_system_env);
    }

    // Lambda para verificação de arquivo e otimização SIMD
    auto try_simd_optimization = [&]() -> std::optional<int> {
        std::filesystem::path file_path(path);
        std::error_code error_code;

        // Early return: arquivo não existe
        if (!std::filesystem::exists(file_path, error_code) || error_code) {
            return std::nullopt;
        }

        // Otimização baseada em dados reais: SIMD só vale a pena para arquivos
        // >50KB Arquivos pequenos: overhead do SIMD supera os benefícios
        auto file_size = std::filesystem::file_size(file_path, error_code);
        if (error_code || file_size < MIN_FILE_SIZE_FOR_SIMD) {
            return std::nullopt; // Usar implementação tradicional
        }

        // Use the legacy pair-returning SIMD API
        auto [simd_error, simd_count] = load_simd_legacy(
            path, {.overwrite_policy = (replace != 0) ? overwrite::replace
                                                      : overwrite::preserve,
                   .apply_to_process = apply_system_env ? process_env_apply::yes
                                                        : process_env_apply::no,
                   .backend = parse_backend::simd});

        if (simd_error == dotenv_error::success) {
            return simd_count;
        }
        return std::nullopt;
    };

    // Tentar otimização SIMD
    if (auto result = try_simd_optimization()) {
        return result.value();
    }
#endif

    // Fallback: implementação tradicional
    return load_traditional_implementation(path, replace, apply_system_env);
}

// Função pública para forçar implementação tradicional (benchmarking)
auto dotenv::load_traditional_raw(std::string_view path, int replace,
                                  bool apply_system_env) noexcept -> int {
    return load_traditional_implementation(path, replace, apply_system_env);
}

auto dotenv::load_with_status(std::string_view path, int replace,
                              bool apply_system_env) noexcept
    -> std::pair<dotenv_error_t, int> {
    int result = load_raw(path, replace, apply_system_env);

    if (result < 0) {
        // Convert negative error codes to enum
        switch (result) {
        case -1:
            return {DOTENV_ERROR_FILE_NOT_FOUND, 0};
        case -2:
            return {DOTENV_ERROR_PERMISSION_DENIED, 0};
        case -3:
            return {DOTENV_ERROR_INVALID_FORMAT, 0};
        case -4:
            return {DOTENV_ERROR_OUT_OF_MEMORY, 0};
        case -5:
            return {DOTENV_ERROR_INVALID_ARGUMENT, 0};
        default:
            return {DOTENV_ERROR_INVALID_FORMAT, 0};
        }
    }

    return {DOTENV_SUCCESS, result};
}

auto dotenv::load_traditional_with_status(std::string_view path, int replace,
                                          bool apply_system_env) noexcept
    -> std::pair<dotenv_error_t, int> {
    int result = load_traditional_raw(path, replace, apply_system_env);

    if (result < 0) {
        // Convert negative error codes to enum
        switch (result) {
        case -1:
            return {DOTENV_ERROR_FILE_NOT_FOUND, 0};
        case -2:
            return {DOTENV_ERROR_PERMISSION_DENIED, 0};
        case -3:
            return {DOTENV_ERROR_INVALID_FORMAT, 0};
        case -4:
            return {DOTENV_ERROR_OUT_OF_MEMORY, 0};
        case -5:
            return {DOTENV_ERROR_INVALID_ARGUMENT, 0};
        default:
            return {DOTENV_ERROR_INVALID_FORMAT, 0};
        }
    }

    return {DOTENV_SUCCESS, result};
}

// Implementação tradicional extraída para reutilização
static auto load_traditional_implementation(std::string_view path, int replace,
                                            bool apply_system_env) noexcept
    -> int {

    // Implementação padrão (fallback ou para arquivos pequenos)
    std::ifstream dotenv((std::filesystem::path(path)));

    if (!dotenv.is_open()) {
        return -1;
    }
    int count = 0;

    size_t line_number = 0;

    std::string line;
    while (std::getline(dotenv, line)) {
        ++line_number;

        // Verificação de limite de linha para evitar DoS
        if (line.size() > MAX_LINE_LENGTH) {
            std::cerr << "Warning: Line " << line_number
                      << " exceeds maximum length (" << MAX_LINE_LENGTH
                      << " chars), skipping" << '\n';
            continue;
        }

        processLine(line, replace, count);
    }

    dotenv.close();

    if (apply_system_env) {
        dotenv::apply_internal_to_process_env(
            (replace != 0) ? dotenv::overwrite::replace
                           : dotenv::overwrite::preserve);
    }

    return count;
}

auto dotenv::get(std::string_view key, std::string_view default_value)
    -> std::string_view {
    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    auto it = envMap.find(key_str);

    if (it != envMap.end()) {
        return it->second.data;
    }

    auto *value = getenv(key_str.c_str());
    return (value != nullptr) ? value : default_value;
}

// ===== CORE C++20 API IMPLEMENTATIONS =====

// Helper function to convert legacy int error codes to dotenv_error
static auto convert_error_code(int error_code) noexcept
    -> dotenv::dotenv_error {
    switch (error_code) {
    case 0: // Success
        return dotenv::dotenv_error::success;
    case -1:
        return dotenv::dotenv_error::file_not_found;
    case -2:
        return dotenv::dotenv_error::permission_denied;
    case -3:
        return dotenv::dotenv_error::invalid_format;
    case -4:
        return dotenv::dotenv_error::out_of_memory;
    case -5:
        return dotenv::dotenv_error::invalid_argument;
    default:
        return dotenv::dotenv_error::invalid_format;
    }
}

// Primary C++20 load API (renamed to legacy pair-returning)
auto dotenv::load_legacy(std::string_view path,
                         const load_options &options) noexcept
    -> std::pair<dotenv::dotenv_error, int> {
    try {
        int replace_flag =
            (options.overwrite_policy == overwrite::replace) ? 1 : 0;
        bool apply_to_env =
            (options.apply_to_process == process_env_apply::yes);

        int result = 0;
        switch (options.backend) {
        case parse_backend::auto_detect:
            result = load_raw(path, replace_flag, apply_to_env);
            break;
        case parse_backend::traditional:
            result = load_traditional_raw(path, replace_flag, apply_to_env);
            break;
#ifdef DOTENV_SIMD_ENABLED
        case parse_backend::simd: {
            auto [simd_error, simd_count] = load_simd_legacy(
                path,
                {.overwrite_policy = (replace_flag == 1) ? overwrite::replace
                                                         : overwrite::preserve,
                 .apply_to_process = apply_to_env ? process_env_apply::yes
                                                  : process_env_apply::no,
                 .backend = parse_backend::simd});

            if (simd_error == dotenv::dotenv_error::success) {
                result = simd_count;
            } else {
                result = load_traditional_raw(path, replace_flag, apply_to_env);
            }
        } break;
#else
        case parse_backend::simd:
            result = load_traditional_raw(path, replace_flag, apply_to_env);
            break;
#endif
        }

        if (result < 0) {
            return {convert_error_code(result), 0};
        }

        return {dotenv::dotenv_error::success, result};
    } catch (const std::exception &) {
        return {dotenv::dotenv_error::out_of_memory, 0};
    }
}

// Traditional backend (C++20 API)
auto dotenv::load_traditional_legacy(std::string_view path,
                                     const load_options &options) noexcept
    -> std::pair<dotenv::dotenv_error, int> {
    try {
        int replace_flag =
            (options.overwrite_policy == overwrite::replace) ? 1 : 0;
        bool apply_to_env =
            (options.apply_to_process == process_env_apply::yes);

        int result = load_traditional_raw(path, replace_flag, apply_to_env);

        if (result < 0) {
            return {convert_error_code(result), 0};
        }

        return {dotenv::dotenv_error::success, result};
    } catch (const std::exception &) {
        return {dotenv::dotenv_error::out_of_memory, 0};
    }
}

#ifdef DOTENV_SIMD_ENABLED
// SIMD backend (legacy pair-returning)
auto dotenv::load_simd_legacy(std::string_view path,
                              const load_options &options) noexcept
    -> std::pair<dotenv::dotenv_error, int> {
    try {
        int replace_flag =
            (options.overwrite_policy == overwrite::replace) ? 1 : 0;
        bool apply_to_env =
            (options.apply_to_process == process_env_apply::yes);

        int result = 0;
        if (simd::is_avx2_available()) {
            // Use traditional implementation for now - SIMD implementation
            // needs to be updated
            result = load_traditional_raw(path, replace_flag, apply_to_env);
        } else {
            result = load_traditional_raw(path, replace_flag, apply_to_env);
        }

        if (result < 0) {
            return {convert_error_code(result), 0};
        }

        return {dotenv::dotenv_error::success, result};
    } catch (const std::exception &) {
        return {dotenv::dotenv_error::out_of_memory, 0};
    }
}
#endif

// ===== C++23 ENHANCED API IMPLEMENTATIONS =====

#if DOTENV_HAS_STD_EXPECTED
// Enhanced C++23 load API with std::expected
std::expected<int, dotenv::dotenv_error>
dotenv::load(std::string_view path, const load_options &options) noexcept {
    auto res = load_legacy(path, options);
    // load_legacy returns pair<dotenv_error, int>
    if (res.first != dotenv::dotenv_error::success) {
        return std::unexpected(res.first);
    }
    return res.second;
}

// Traditional backend (C++23 enhanced API)
std::expected<int, dotenv::dotenv_error>
dotenv::load_traditional(std::string_view path,
                         const load_options &options) noexcept {
    auto res = load_traditional_legacy(path, options);
    if (res.first != dotenv::dotenv_error::success) {
        return std::unexpected(res.first);
    }
    return res.second;
}

#ifdef DOTENV_SIMD_ENABLED
// SIMD backend (C++23 enhanced API)
std::expected<int, dotenv::dotenv_error>
dotenv::load_simd(std::string_view path, const load_options &options) noexcept {
    auto res = load_simd_legacy(path, options);
    if (res.first != dotenv::dotenv_error::success) {
        return std::unexpected(res.first);
    }
    return res.second;
}
#endif
#endif // DOTENV_HAS_STD_EXPECTED

// Deprecated C-style int wrappers (forward to raw implementations)
int dotenv::load(std::string_view path, int replace,
                 bool apply_system_env) noexcept {
    return load_raw(path, replace, apply_system_env);
}

int dotenv::load_traditional(std::string_view path, int replace,
                             bool apply_system_env) noexcept {
    return load_traditional_raw(path, replace, apply_system_env);
}

// ===== VARIABLE ACCESS API IMPLEMENTATIONS =====

auto dotenv::value(std::string_view key, std::string_view default_value)
    -> std::string {
    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    auto it = envMap.find(key_str);

    if (it != envMap.end()) {
        return it->second.data;
    }

    auto *value = getenv(key_str.c_str());
    return (value != nullptr) ? std::string(value) : std::string(default_value);
}

auto dotenv::value_or(std::string_view key, std::string_view fallback_value)
    -> std::string {
    return value(key, fallback_value);
}

auto dotenv::try_value(std::string_view key) noexcept
    -> std::optional<std::string> {
    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    auto it = envMap.find(key_str);

    if (it != envMap.end()) {
        return it->second.data;
    }

    auto *value = getenv(key_str.c_str());
    if (value != nullptr) {
        return std::string(value);
    }

    return std::nullopt;
}

auto dotenv::contains(std::string_view key) -> bool {
    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    auto it = envMap.find(key_str);

    if (it != envMap.end()) {
        return true;
    }

    return getenv(key_str.c_str()) != nullptr;
}

void dotenv::save_to_file(std::string_view path) {
    std::ofstream output_file{std::string(path)};
    if (!output_file.is_open()) {
        throw std::runtime_error("Cannot create output file: " +
                                 std::string(path));
    }

    std::lock_guard<std::mutex> lock(envMapMutex);
    for (const auto &[key, value] : envMap) {
        output_file << key << "=" << value.data << "\n";
    }
}

#if DOTENV_HAS_STD_EXPECTED
std::expected<std::string, dotenv::dotenv_error>
dotenv::value_expected(std::string_view key) {
    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    auto it = envMap.find(key_str);

    if (it != envMap.end()) {
        return it->second.data;
    }

    auto *value = getenv(key_str.c_str());
    if (value != nullptr) {
        return std::string(value);
    }

    return std::unexpected(dotenv::dotenv_error::key_not_found);
}
#endif // DOTENV_HAS_STD_EXPECTED

// ===== LEGACY API COMPATIBILITY WRAPPERS =====
// Note: The load() and load_traditional() functions with int/bool parameters
// are already implemented above in the main implementation section

void dotenv::set(std::string_view key, std::string_view value,
                 overwrite overwrite_policy) {
    std::lock_guard<std::mutex> lock(envMapMutex);
    std::string key_str(key);

    if (overwrite_policy == overwrite::replace) {
        envMap.insert_or_assign(std::move(key_str),
                                ValueStruct(std::string(value), true));
    } else {
        // Se overwrite::preserve, só insere se não existir
        envMap.emplace(std::move(key_str),
                       ValueStruct(std::string(value), true));
    }
}

void dotenv::unset(std::string_view key) {
    std::lock_guard<std::mutex> lock(envMapMutex);
    envMap.erase(std::string(key));
}
