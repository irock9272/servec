# HTTP Server

A simple, lightweight HTTP server written in C that serves static files.

## Overview

This HTTP server:
- Listens on port 8080 (configurable via command line)
- Serves static files from the current directory
- Supports common MIME types (HTML, CSS, JS, images, etc.)
- Handles basic GET requests
- Returns 404 for invalid paths or unsupported methods

## Files

| File | Description |
|------|-------------|
| `http_server` | Compiled HTTP server executable |
| `http_server.c` | C source code (177 lines) |
| `README.md` | This documentation file |

## Building

```bash
gcc -o http_server http_server.c
```

## Usage

```bash
# Default port (8080)
./http_server

# Custom port
./http_server 3000
```

---

## Code Breakdown

### 1. Headers and Constants

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 4096
```

**Purpose:** Includes necessary system libraries for socket programming, file I/O, and string manipulation.

---

### 2. MIME Type Detection

```c
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
```

**Purpose:** Determines the correct `Content-Type` header based on file extension. This ensures browsers render files correctly.

---

### 3. HTTP Response Sender

```c
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
```

**Purpose:** Constructs and sends a proper HTTP response with headers and body. The response includes:
- HTTP status line (e.g., "200 OK", "404 Not Found")
- Content-Type header
- Content-Length header
- Connection header (set to close for simplicity)

---

### 4. 404 Error Handler

```c
void send_404(int client_socket) {
    const char* body = "<html><body><h1>404 Not Found</h1></body></html>";
    send_response(client_socket, "404 Not Found", "text/html", body, strlen(body));
}
```

**Purpose:** Sends a standardized 404 response for invalid requests or missing files.

---

### 5. Request Handler

```c
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
    
    printf("%s %s %s\n", method, path, protocol);
    
    // Only handle GET requests
    if (strcmp(method, "GET") != 0) {
        send_404(client_socket);
        close(client_socket);
        return;
    }
    
    // Default to index.html if root is requested
    if (strcmp(path, "/") == 0) {
        strcpy(path, "/index.html");
    }
    
    // Prevent directory traversal attacks
    if (strstr(path, "..") != NULL) {
        send_404(client_socket);
        close(client_socket);
        return;
    }
    
    // Remove leading slash for file path
    char* filepath = path + 1;
    
    // Try to open the file
    int file = open(filepath, O_RDONLY);
    if (file < 0) {
        send_404(client_socket);
        close(client_socket);
        return;
    }
    
    // Get file size
    struct stat stat_buf;
    if (fstat(file, &stat_buf) < 0) {
        close(file);
        send_404(client_socket);
        close(client_socket);
        return;
    }
    
    // Read file content
    char* file_content = malloc(stat_buf.st_size);
    if (!file_content) {
        close(file);
        send_404(client_socket);
        close(client_socket);
        return;
    }
    
    size_t bytes_file = read(file, file_content, stat_buf.st_size);
    close(file);
    
    // Send response
    const char* mime_type = get_mime_type(filepath);
    send_response(client_socket, "200 OK", mime_type, file_content, bytes_file);
    
    free(file_content);
    close(client_socket);
}
```

**Purpose:** Core request handling logic:
1. Reads the HTTP request from the client socket
2. Parses the method, path, and protocol
3. Validates it's a GET request
4. Handles root path by defaulting to `index.html`
5. **Security:** Blocks directory traversal attacks (`..`)
6. Opens and reads the requested file
7. Sends the file with appropriate MIME type

---

### 6. Main Server Loop

```c
int main(int argc, char* argv[]) {
    int port = PORT;
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
```

**Purpose:** Initializes and runs the server:
1. Creates a TCP socket
2. Sets `SO_REUSEADDR` to allow quick restarts
3. Binds to all interfaces on the specified port
4. Listens for incoming connections (backlog of 10)
5. Accepts connections in an infinite loop
6. Delegates each connection to `handle_request()`

---

## Security Considerations

| Feature | Implementation |
|---------|----------------|
| Directory Traversal | Blocked by checking for `..` in paths |
| Buffer Overflow | Fixed-size buffers with bounds in `sscanf` |
| Memory Leaks | All allocated memory is freed after use |

## Limitations

- Only supports GET requests
- No HTTPS/TLS support
- Single-threaded (handles one request at a time)
- No keep-alive connections
- No request body handling (no POST/PUT)

## Example Session

```bash
$ ./http_server 8080
Server running on http://localhost:8080
Press Ctrl+C to stop
GET /index.html HTTP/1.1
GET /style.css HTTP/1.1
GET /missing.html HTTP/1.1
```

Then in a browser or with curl:
```bash
curl http://localhost:8080/index.html
```
