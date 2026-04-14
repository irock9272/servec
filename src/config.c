#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#define CONFIG_DIR_NAME ".config/servec"
#define CONFIG_FILE_NAME "config"
#define MAX_LINE_LEN 1024

static char config_dir_path[MAX_PATH_LEN];
static bool config_dir_initialized = false;

const char* get_config_dir(void) {
    if (!config_dir_initialized) {
        const char* home = getenv("HOME");
        if (!home) {
            home = getpwuid(getuid())->pw_dir;
        }
        if (home) {
            snprintf(config_dir_path, sizeof(config_dir_path), "%s/%s", home, CONFIG_DIR_NAME);
        } else {
            strcpy(config_dir_path, CONFIG_DIR_NAME);
        }
        config_dir_initialized = true;
    }
    return config_dir_path;
}

static bool ensure_config_dir(void) {
    const char* dir = get_config_dir();
    struct stat st;
    if (stat(dir, &st) != 0) {
        // Directory doesn't exist, create it
        if (mkdir(dir, 0755) != 0) {
            return false;
        }
    }
    return true;
}

static char* trim(char* str) {
    // Trim leading whitespace
    while (*str == ' ' || *str == '\t') str++;
    
    // Trim trailing whitespace
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    return str;
}

bool load_config(ServerConfig* config) {
    // Set defaults
    config->port = DEFAULT_PORT;
    strcpy(config->root_dir, DEFAULT_ROOT_DIR);
    config->error_404[0] = '\0';
    config->error_500[0] = '\0';
    config->use_custom_404 = false;
    config->use_custom_500 = false;
    
    // Seed default allowed extensions
    const char* defaults[] = {".html", ".css", ".js", ".png", ".jpg", ".jpeg", ".gif"};
    int num_defaults = 7;
    for (int i = 0; i < num_defaults; i++) {
        strncpy(config->allowed_exts[i], defaults[i], 15);
        config->allowed_exts[i][15] = '\0';
    }
    config->allowed_exts_count = num_defaults;
    config->use_allowed_exts = true;
    
    // Build config file path
    char config_path[MAX_PATH_LEN];
    snprintf(config_path, sizeof(config_path), "%s/%s", get_config_dir(), CONFIG_FILE_NAME);
    
    FILE* file = fopen(config_path, "r");
    if (!file) {
        // Config file doesn't exist, use defaults
        return false;
    }
    
    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        char* trimmed = trim(line);
        if (trimmed[0] == '#' || trimmed[0] == '\0') {
            continue;
        }
        
        // Parse key=value
        char* eq = strchr(trimmed, '=');
        if (!eq) continue;
        
        *eq = '\0';
        char* key = trim(trimmed);
        char* value = trim(eq + 1);
        
        if (strcmp(key, "port") == 0) {
            config->port = atoi(value);
        } else if (strcmp(key, "root_dir") == 0) {
            strncpy(config->root_dir, value, MAX_PATH_LEN - 1);
            config->root_dir[MAX_PATH_LEN - 1] = '\0';
        } else if (strcmp(key, "error_404") == 0) {
            strncpy(config->error_404, value, MAX_PATH_LEN - 1);
            config->error_404[MAX_PATH_LEN - 1] = '\0';
            config->use_custom_404 = true;
        } else if (strcmp(key, "error_500") == 0) {
            strncpy(config->error_500, value, MAX_PATH_LEN - 1);
            config->error_500[MAX_PATH_LEN - 1] = '\0';
            config->use_custom_500 = true;
        } else if (strcmp(key, "extra_exts") == 0) {
            char* exts_copy = strdup(value);
            char* token = strtok(exts_copy, ",");
            while (token != NULL && config->allowed_exts_count < 20) {
                char* trimmed_ext = trim(token);
                if (trimmed_ext[0] != '\0') {
                    strncpy(config->allowed_exts[config->allowed_exts_count], trimmed_ext, 15);
                    config->allowed_exts[config->allowed_exts_count][15] = '\0';
                    config->allowed_exts_count++;
                }
                token = strtok(NULL, ",");
            }
            free(exts_copy);
        }
    }
    
    fclose(file);
    return true;
}
