#pragma once

#ifdef __cplusplus
#include "dotenv.hpp"
extern "C" {
#endif

#include "dotenv_errors.h" /* Shared error codes */
#include <stdbool.h>       /* for bool (C99+) */
#include <stddef.h>        /* for size_t */

/* Load options structure for advanced configuration */
typedef struct {
    int replace_existing;   /* 0=preserve existing, 1=replace existing */
    int apply_to_system;    /* 0=internal only, 1=apply to system environment */
    size_t max_line_length; /* Maximum allowed line length (0=use default) */
    size_t max_key_length;  /* Maximum allowed key length (0=use default) */
    size_t max_value_length; /* Maximum allowed value length (0=use default) */
} dotenv_load_options_t;

/* Statistics structure for load operations */
typedef struct {
    int variables_loaded;   /* Number of variables successfully loaded */
    int variables_skipped;  /* Number of variables skipped (comments, invalid,
                               etc.) */
    int variables_rejected; /* Number of variables rejected (security, limits,
                               etc.) */
    int lines_processed;    /* Total number of lines processed */
} dotenv_load_stats_t;

/* ==== Core Loading Functions ==== */

/**
 * @brief Load environment variables from a .env file (simple interface)
 * @param path Path to the .env file (NULL for ".env")
 * @param replace 0=preserve existing variables, 1=replace existing
 * @param apply_system_env 0=internal only, 1=apply to system environment
 * @return Number of variables loaded on success, negative error code on failure
 * @note This is the basic interface compatible with the previous API
 */
int dotenv_load(const char *path, int replace, int apply_system_env);

/**
 * @brief Load environment variables with advanced options
 * @param path Path to the .env file (NULL for ".env")
 * @param options Pointer to load options structure (NULL for defaults)
 * @param stats Pointer to statistics structure (NULL if not needed)
 * @return DOTENV_SUCCESS on success, negative error code on failure
 */
dotenv_error_t dotenv_load_ex(const char *path,
                              const dotenv_load_options_t *options,
                              dotenv_load_stats_t *stats);

/**
 * @brief Force traditional loading (no SIMD optimizations)
 * @param path Path to the .env file (NULL for ".env")
 * @param replace 0=preserve existing, 1=replace existing
 * @param apply_system_env 0=internal only, 1=apply to system environment
 * @return Number of variables loaded on success, negative error code on failure
 */
int dotenv_load_traditional(const char *path, int replace,
                            int apply_system_env);

/* ==== Variable Access Functions ==== */

/**
 * @brief Get environment variable value
 * @param key Variable name to retrieve
 * @param default_value Default value if variable not found (can be NULL)
 * @return Variable value or default_value, never returns NULL (returns "" if
 * default is NULL)
 * @note Returned pointer is valid until next dotenv operation or program exit
 */
const char *dotenv_get(const char *key, const char *default_value);

/**
 * @brief Get environment variable value with buffer
 * @param key Variable name to retrieve
 * @param buffer Buffer to store the value
 * @param buffer_size Size of the buffer
 * @return DOTENV_SUCCESS on success, DOTENV_ERROR_BUFFER_TOO_SMALL if buffer
 * too small
 */
dotenv_error_t dotenv_get_buffer(const char *key, char *buffer,
                                 size_t buffer_size);

/**
 * @brief Check if environment variable exists
 * @param key Variable name to check
 * @return 1 if variable exists, 0 if not found
 */
int dotenv_has(const char *key);

/**
 * @brief Get environment variable as integer
 * @param key Variable name to retrieve
 * @param default_value Default value if variable not found or invalid
 * @return Variable value as integer or default_value
 */
int dotenv_get_int(const char *key, int default_value);

/**
 * @brief Get environment variable as long
 * @param key Variable name to retrieve
 * @param default_value Default value if variable not found or invalid
 * @return Variable value as long or default_value
 */
long dotenv_get_long(const char *key, long default_value);

/**
 * @brief Get environment variable as double
 * @param key Variable name to retrieve
 * @param default_value Default value if variable not found or invalid
 * @return Variable value as double or default_value
 */
double dotenv_get_double(const char *key, double default_value);

/**
 * @brief Get environment variable as boolean
 * @param key Variable name to retrieve
 * @param default_value Default value if variable not found or invalid
 * @return 1 for "true", "1", "yes", "on", 0 for "false", "0", "no", "off",
 * default_value for others
 */
int dotenv_get_bool(const char *key, int default_value);

/* ==== Variable Modification Functions ==== */

/**
 * @brief Set environment variable
 * @param key Variable name to set
 * @param value Variable value to set
 * @param replace 0=don't replace if exists, 1=replace if exists
 * @return DOTENV_SUCCESS on success, negative error code on failure
 */
dotenv_error_t dotenv_set(const char *key, const char *value, int replace);

/**
 * @brief Unset (remove) environment variable
 * @param key Variable name to remove
 * @return DOTENV_SUCCESS on success, negative error code on failure
 */
dotenv_error_t dotenv_unset(const char *key);

/* ==== File Operations ==== */

/**
 * @brief Save current environment variables to file
 * @param path Path to save the .env file (NULL for ".env")
 * @return DOTENV_SUCCESS on success, negative error code on failure
 */
dotenv_error_t dotenv_save(const char *path);

/* ==== Utility Functions ==== */

/**
 * @brief Get default load options structure
 * @param options Pointer to options structure to initialize
 */
void dotenv_get_default_options(dotenv_load_options_t *options);

/**
 * @brief Get library version information
 * @param major Pointer to store major version (can be NULL)
 * @param minor Pointer to store minor version (can be NULL)
 * @param patch Pointer to store patch version (can be NULL)
 * @return Version string in format "major.minor.patch"
 */
const char *dotenv_get_version(int *major, int *minor, int *patch);

/* ==== Advanced Functions ==== */

/**
 * @brief Iterator function type for enumerating variables
 * @param key Variable name
 * @param value Variable value
 * @param user_data User-provided data pointer
 * @return 0 to continue iteration, non-zero to stop
 */
typedef int (*dotenv_iterator_t)(const char *key, const char *value,
                                 void *user_data);

/**
 * @brief Enumerate all loaded environment variables
 * @param iterator Function to call for each variable
 * @param user_data Pointer passed to iterator function
 * @return Number of variables enumerated, or negative error code
 */
int dotenv_enumerate(dotenv_iterator_t iterator, void *user_data);

/**
 * @brief Clear all internally stored variables
 * @param clear_system 0=keep system environment, 1=also clear from system
 * @return DOTENV_SUCCESS on success, negative error code on failure
 */
dotenv_error_t dotenv_clear(int clear_system);

/**
 * @brief Get human-readable error message for error code
 * @param error_code The error code to get message for
 * @return Human-readable error message string
 */
const char *dotenv_get_error_message(dotenv_error_t error_code);

#ifdef __cplusplus
}
#endif
