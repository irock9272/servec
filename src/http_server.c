#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "config.h"
#include "safety.h"

#define BUFFER_SIZE 4096

// Global configuration
static ServerConfig g_config;

const char* get_mime_type(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    return "application/octet-stream";
}

void send_response(int client_socket, const char* status, const char* content_type, const char* body, size_t body_len) {
    char header[BUFFER_SIZE];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, content_type, body_len);
    
    write(client_socket, header, header_len);
    if (body && body_len > 0) {
        write(client_socket, body, body_len);
    }
}

char* load_file_content(const char* filepath, size_t* out_size) {
    int file = open(filepath, O_RDONLY);
    if (file < 0) {
        return NULL;
    }
    
    struct stat stat_buf;
    if (fstat(file, &stat_buf) < 0) {
        close(file);
        return NULL;
    }
    
    char* content = malloc(stat_buf.st_size);
    if (!content) {
        close(file);
        return NULL;
    }
    
    size_t bytes_read = read(file, content, stat_buf.st_size);
    close(file);
    
    if (out_size) *out_size = bytes_read;
    return content;
}

void send_404(int client_socket) {
    char* body = NULL;
    size_t body_len = 0;
    const char* content_type = "text/html";
    
    // Try to load custom 404 page
    if (g_config.use_custom_404 && g_config.error_404[0]) {
        body = load_file_content(g_config.error_404, &body_len);
    }
    
    // Use default if custom not available
    if (!body) {
        const char* default_body = "<html><body><h1>404 Not Found</h1></body></html>";
        body = strdup(default_body);
        body_len = strlen(default_body);
    }
    
    send_response(client_socket, "404 Not Found", content_type, body, body_len);
    free(body);
}

void send_500(int client_socket) {
    char* body = NULL;
    size_t body_len = 0;
    const char* content_type = "text/html";
    
    // Try to load custom 500 page
    if (g_config.use_custom_500 && g_config.error_500[0]) {
        body = load_file_content(g_config.error_500, &body_len);
    }
    
    // Use default if custom not available
    if (!body) {
        const char* default_body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        body = strdup(default_body);
        body_len = strlen(default_body);
    }
    
    send_response(client_socket, "500 Internal Server Error", content_type, body, body_len);
    free(body);
}

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    
    buffer[bytes_read] = '\0';
    
    // Parse the request path
    char method[16], path[256], protocol[16];
    if (sscanf(buffer, "%15s %255s %15s", method, path, protocol) != 3) {
        send_404(client_socket);
        close(client_socket);
        return;
    }
    
    // Default to index.html if root is requested
    if (strcmp(path, "/") == 0) {
        strcpy(path, "/index.html");
    }
    
    if (!is_path_safe(path) || !is_ext_allowed(path, &g_config)) {
        send_404(client_socket);
        close(client_socket);
        return;
    }
    
    printf("%s %s %s\n", method, path, protocol);
    
    // Only handle GET requests
    if (strcmp(method, "GET") != 0) {
        send_404(client_socket);
        close(client_socket);
        return;
    }
    
    // Prevent directory traversal attacks
    if (strstr(path, "..") != NULL) {
        send_404(client_socket);
        close(client_socket);
        return;
    }
    
    // Build full filepath with root_dir
    char filepath[MAX_PATH_LEN];
    snprintf(filepath, sizeof(filepath), "%s%s", g_config.root_dir, path);
    
    // Try to open the file
    char* file_content = load_file_content(filepath, NULL);
    if (!file_content) {
        send_404(client_socket);
        close(client_socket);
        return;
    }
    
    // Get file size for response
    struct stat stat_buf;
    if (stat(filepath, &stat_buf) < 0) {
        free(file_content);
        send_500(client_socket);
        close(client_socket);
        return;
    }
    
    // Send response
    const char* mime_type = get_mime_type(filepath);
    send_response(client_socket, "200 OK", mime_type, file_content, stat_buf.st_size);
    
    free(file_content);
    close(client_socket);
}

int main(int argc, char* argv[]) {
    // Load configuration
    bool config_loaded = load_config(&g_config);
    if (config_loaded) {
        printf("Configuration loaded from: %s\n", get_config_dir());
    } else {
        printf("Using default configuration\n");
    }
    
    // Allow command line override for port
    int port = g_config.port;
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(1);
    }
    
    // Allow socket reuse
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(1);
    }
    
    if (listen(server_socket, 10) < 0) {
        perror("listen");
        exit(1);
    }
    
    printf("Server running on http://localhost:%d\n", port);
    printf("Serving files from: %s\n", g_config.root_dir);
    printf("Press Ctrl+C to stop\n");
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        
        handle_request(client_socket);
    }
    
    close(server_socket);
    return 0;
}
