#ifndef SAFETY_H
#define SAFETY_H

#include <stdbool.h>
#include "config.h"

bool is_path_safe(const char* path);
bool is_ext_allowed(const char* path, const ServerConfig* config);

#endif