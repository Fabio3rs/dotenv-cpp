#pragma once
#ifdef __cplusplus
#include "dotenv.hpp"
extern "C" {
#endif

int dotenv_load(const char *path, int replace);
const char *dotenv_get(const char *key, const char *default_value);
void dotenv_save(const char *path);

#ifdef __cplusplus
}
#endif
