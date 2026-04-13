#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define DEFAULT_PORT 6969
#define DEFAULT_ROOT_DIR "."
#define MAX_PATH_LEN 512

typedef struct {
    int port;
    char root_dir[MAX_PATH_LEN];
    char error_404[MAX_PATH_LEN];
    char error_500[MAX_PATH_LEN];
    bool use_custom_404;
    bool use_custom_500;
} ServerConfig;

// Load configuration from ~/.config/servec/config
// Returns true if config loaded successfully, false if using defaults
bool load_config(ServerConfig* config);

// Get config directory path
const char* get_config_dir(void);

#endif // CONFIG_H
