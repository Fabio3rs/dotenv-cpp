#include "dotenv.hpp"
#include "dotenv.h"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

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
static int set_env(const char *key, const char *value, int replace) {
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
static std::string_view trim(std::string_view str) {
    const auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string_view::npos) {
        return {};
    }
    const auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// Função para processar aspas e escape básico
static std::string process_quoted_value(std::string_view value) {
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

// Validação de chave (deve ser um identificador válido)
static bool is_valid_key(std::string_view key) {
    if (key.empty() || key.size() > MAX_KEY_LENGTH) {
        return false;
    }

    // Primeira char deve ser letra ou underscore
    if (!std::isalpha(key[0]) && key[0] != '_') {
        return false;
    }

    // Restante deve ser alfanumérico ou underscore
    for (size_t i = 1; i < key.size(); ++i) {
        if (!std::isalnum(key[i]) && key[i] != '_') {
            return false;
        }
    }

    return true;
}

namespace {

// Mapa seguro que possui a memória das strings e sincronização para threads
std::unordered_map<std::string, std::string> envMap;
std::mutex envMapMutex;

// Conjunto para rastrear chaves carregadas do .env (para save seguro)
std::unordered_set<std::string> managedKeys;

} // namespace

extern "C" {
int dotenv_load(const char *path, int replace) {
    return dotenv::load(path, replace);
}

const char *dotenv_get(const char *key, const char *default_value) {
    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    auto it = envMap.find(key_str);

    if (it != envMap.end()) {
        return it->second.c_str();
    }

    auto value = getenv(key);
    return value ? value : default_value;
}

void dotenv_save(const char *path) { dotenv::save(path); }
}

int dotenv::load(std::string_view path, int replace) noexcept {
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
                      << " chars), skipping" << std::endl;
            continue;
        }

        // Trim da linha
        auto trimmed_line = trim(line);

        // Ignorar linhas vazias
        if (trimmed_line.empty()) {
            continue;
        }

        // Ignorar comentários (linhas que começam com #)
        if (trimmed_line[0] == '#') {
            continue;
        }

        // Buscar primeiro '=' que não esteja entre aspas
        size_t eq_pos = std::string_view::npos;
        bool in_quotes = false;
        char quote_char = '\0';

        for (size_t i = 0; i < trimmed_line.size(); ++i) {
            char c = trimmed_line[i];
            if (!in_quotes && (c == '"' || c == '\'')) {
                in_quotes = true;
                quote_char = c;
            } else if (in_quotes && c == quote_char &&
                       (i == 0 || trimmed_line[i - 1] != '\\')) {
                in_quotes = false;
                quote_char = '\0';
            } else if (!in_quotes && c == '=' &&
                       eq_pos == std::string_view::npos) {
                eq_pos = i;
                break;
            }
        }

        // Se não encontrou '=', ignora a linha
        if (eq_pos == std::string_view::npos) {
            std::cerr << "Warning: Line " << line_number
                      << " has no '=' separator, skipping: "
                      << std::quoted(std::string(trimmed_line)) << std::endl;
            continue;
        }

        // Extrair chave e valor
        auto raw_key = trim(trimmed_line.substr(0, eq_pos));
        auto raw_value = trimmed_line.substr(eq_pos + 1);

        // Validar chave
        if (!is_valid_key(raw_key)) {
            std::cerr << "Warning: Line " << line_number
                      << " has invalid key format, skipping: "
                      << std::quoted(std::string(raw_key)) << std::endl;
            continue;
        }

        // Processar valor (remover aspas e escape)
        auto processed_value = process_quoted_value(raw_value);

        // Verificar limite de valor
        if (processed_value.size() > MAX_VALUE_LENGTH) {
            std::cerr << "Warning: Line " << line_number
                      << " has value exceeding maximum length ("
                      << MAX_VALUE_LENGTH << " chars), truncating" << std::endl;
            processed_value.resize(MAX_VALUE_LENGTH);
        }

        // Definir variável de ambiente
        std::string key_str(raw_key);
        int res = set_env(key_str.c_str(), processed_value.c_str(), replace);

        if (res != 0) {
            std::cerr << "Failed to set environment variable: "
                      << std::quoted(key_str) << std::endl;
        } else {
            count++;
            // Rastrear chaves carregadas do .env para save seguro
            std::lock_guard<std::mutex> lock(envMapMutex);
            managedKeys.insert(key_str);
            envMap[key_str] = processed_value;
        }
    }

    dotenv.close();

    return count;
}

std::string_view dotenv::get(std::string_view key,
                             std::string_view default_value) {
    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    auto it = envMap.find(key_str);

    if (it != envMap.end()) {
        return it->second;
    }

    auto value = getenv(key_str.c_str());
    return value ? value : default_value;
}

std::string dotenv::get_string(std::string_view key,
                               std::string_view default_value) {
    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    auto it = envMap.find(key_str);

    if (it != envMap.end()) {
        return it->second;
    }

    auto value = getenv(key_str.c_str());
    return value ? std::string(value) : std::string(default_value);
}

bool dotenv::has(std::string_view key) {
    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    if (envMap.find(key_str) != envMap.end()) {
        return true;
    }

    return getenv(key_str.c_str()) != nullptr;
}

void dotenv::set(std::string_view key, std::string_view value, bool replace) {
    std::string key_str(key);
    std::string value_str(value);

    // Se replace=false, verificar se a variável já existe
    if (!replace) {
        auto existing_value = getenv(key_str.c_str());
        if (existing_value != nullptr) {
            // Variável já existe e replace=false, não fazer nada
            return;
        }
    }

    int result = set_env(key_str.c_str(), value_str.c_str(), replace ? 1 : 0);

    if (result == 0) {
        // Atualização O(1) - apenas a entrada específica
        std::lock_guard<std::mutex> lock(envMapMutex);
        envMap[key_str] = value_str;
        // Rastrear chaves definidas via set também para save
        managedKeys.insert(key_str);
    }
}

void dotenv::unset(std::string_view key) {
    std::string key_str(key);

#ifdef _WIN32
    // No Windows, usar _putenv com string vazia
    std::string env_str = key_str + "=";
    _putenv(env_str.c_str());
#else
    // Em sistemas POSIX, usar unsetenv
    unsetenv(key_str.c_str());
#endif

    // Remover do mapa interno
    std::lock_guard<std::mutex> lock(envMapMutex);
    envMap.erase(key_str);
    managedKeys.erase(key_str);
}

void dotenv::save(std::string_view path) {
    std::ofstream dotenv((std::filesystem::path(path)));

    if (!dotenv.is_open()) {
        return;
    }

    std::lock_guard<std::mutex> lock(envMapMutex);

    // Salvar apenas chaves gerenciadas (carregadas do .env) para segurança
    for (const auto &key : managedKeys) {
        auto it = envMap.find(key);
        if (it != envMap.end()) {
            dotenv << key << "=" << it->second << '\n';
        }
    }

    dotenv.close();
}

std::optional<std::string> dotenv::get_optional_string(std::string_view key) {
    auto value = get_string(key, "");

    if (value.empty()) {
        // Verificar se a chave realmente existe (valor vazio vs inexistente)
        if (!has(key)) {
            return std::nullopt;
        }
    }

    return value;
}
