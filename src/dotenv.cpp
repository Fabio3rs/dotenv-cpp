#include "dotenv.hpp"
#include "dotenv.h"
#include <cctype>
#include <cstddef>
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
auto dotenv_load(const char *path, int replace, int apply_system_env) -> int {
    return dotenv::load(path, replace, apply_system_env != 0);
}

auto dotenv_get(const char *key, const char *default_value) -> const char * {
    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    auto it = envMap.find(key_str);

    if (it != envMap.end()) {
        return it->second.data.c_str();
    }

    auto *value = getenv(key);
    return (value != nullptr) ? value : default_value;
}

void dotenv_save(const char *path) { dotenv::save(path); }
}

void dotenv::write_system_env_from_env_map(int replace) {
    std::lock_guard<std::mutex> lock(envMapMutex);
    for (const auto &pair : envMap) {
        set_env(pair.first.c_str(), pair.second.data.c_str(), replace);
    }
}

auto dotenv::load(std::string_view path, int replace,
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

        return load_simd(path, replace, apply_system_env);
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
auto dotenv::load_traditional(std::string_view path, int replace,
                              bool apply_system_env) noexcept -> int {
    return load_traditional_implementation(path, replace, apply_system_env);
}

// Implementação tradicional extraída para reutilização
static auto
load_traditional_implementation(std::string_view path, int replace,
                                bool apply_system_env) noexcept -> int {

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
        dotenv::write_system_env_from_env_map(replace);
    }

    return count;
}

auto dotenv::get(std::string_view key,
                 std::string_view default_value) -> std::string_view {
    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    auto it = envMap.find(key_str);

    if (it != envMap.end()) {
        return it->second.data;
    }

    auto *value = getenv(key_str.c_str());
    return (value != nullptr) ? value : default_value;
}

auto dotenv::get_string(std::string_view key,
                        std::string_view default_value) -> std::string {
    std::lock_guard<std::mutex> lock(envMapMutex);

    std::string key_str(key);
    auto it = envMap.find(key_str);

    if (it != envMap.end()) {
        return it->second.data;
    }

    auto *value = getenv(key_str.c_str());
    return (value != nullptr) ? std::string(value) : std::string(default_value);
}

auto dotenv::has(std::string_view key) -> bool {
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
        auto *existing_value = getenv(key_str.c_str());
        if (existing_value != nullptr) {
            // Variável já existe e replace=false, não fazer nada
            return;
        }
    }

    int result = set_env(key_str.c_str(), value_str.c_str(), replace ? 1 : 0);

    if (result == 0) {
        // Atualização O(1) - apenas a entrada específica
        std::lock_guard<std::mutex> lock(envMapMutex);
        envMap[key_str] = ValueStruct(value_str, true);
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
}

void dotenv::save(std::string_view path) {
    std::ofstream dotenv((std::filesystem::path(path)));

    if (!dotenv.is_open()) {
        return;
    }

    std::lock_guard<std::mutex> lock(envMapMutex);

    // Salvar apenas chaves gerenciadas (carregadas do .env) para segurança
    for (const auto &[key, value] : envMap) {
        if (value.managedKey) {
            dotenv << key << "=" << value.data << '\n';
        }
    }

    dotenv.close();
}

auto dotenv::get_optional_string(std::string_view key)
    -> std::optional<std::string> {
    auto value = get_string(key, "");

    if (value.empty()) {
        // Verificar se a chave realmente existe (valor vazio vs inexistente)
        if (!has(key)) {
            return std::nullopt;
        }
    }

    return value;
}

#ifdef DOTENV_SIMD_ENABLED
auto dotenv::load_simd(std::string_view path, int replace,
                       bool apply_system_env) noexcept -> int {
    // Verificar se AVX2 está disponível
    if (!simd::is_avx2_available()) {
        // Fallback para implementação padrão
        return load_traditional(path, replace, apply_system_env);
    }

    int count = 0;
    // Usar callback-based SIMD com memory-mapping (versão mais rápida)
    try {
        mapped_file mapped{path};
        if (!mapped.is_mapped()) {
            return -1; // Arquivo não pode ser mapeado
        }

        auto file_view = mapped.view();
        if (file_view.empty()) {
            return 0; // Arquivo vazio
        }

        // Lambda callback para processar cada linha (mesma lógica da
        // auto-detecção)
        auto process_line = [&](size_t /* line_idx */, std::string_view line) {
            // Trim da linha
            processLine(line, replace, count);
        };

        // Processar com SIMD + callback (versão mais rápida)
        [[maybe_unused]] auto processed_lines =
            simd::process_lines_avx2(file_view, '\n', process_line);

    } catch (...) {
        // Em caso de erro, fallback para implementação tradicional
        return load_traditional_implementation(path, replace, apply_system_env);
    }

    if (apply_system_env) {
        dotenv::write_system_env_from_env_map(replace);
    }
    return count;
}
#endif
