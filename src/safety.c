#include "safety.h"
#include <string.h>

// Blocked path prefixes/names regardless of extension whitelist
static const char* BLOCKED_PATHS[] = {
    ".git",
    ".env",
    "/Makefile",
    "/src",
    "/build",
    NULL
};

bool is_path_safe(const char* path) {
    // Block directory traversal
    if (strstr(path, "..") != NULL) return false;

    // Block sensitive paths
    for (int i = 0; BLOCKED_PATHS[i] != NULL; i++) {
        if (strstr(path, BLOCKED_PATHS[i]) != NULL) return false;
    }

    return true;
}

bool is_ext_allowed(const char* path, const ServerConfig* config) {
    if (!config->use_allowed_exts) return true;

    const char* ext = strrchr(path, '.');
    if (!ext) return false;  // no extension = deny

    for (int i = 0; i < config->allowed_exts_count; i++) {
        if (strcmp(ext, config->allowed_exts[i]) == 0) return true;
    }
    return false;
}