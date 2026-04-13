# servec

A simple, lightweight HTTP server written in C with configuration file support.

## Features

- 🚀 Fast and lightweight
- 📁 MIME type detection for common file types
- 🔒 Directory traversal protection
- ⚙️ Configuration file support (`~/.config/servec/config`)
- 🎨 Custom error pages (404, 500)
- 🔌 Configurable port and root directory

## Quick Start

```bash
# Build
make

# Run with default settings (port 6969)
./build/http_server

# Run on a specific port
./build/http_server 8080
```

## Configuration

Create a configuration file at `~/.config/servec/config`:

```ini
# servec configuration file
port = 6969
root_dir = .
error_404 = ./error_404.html
error_500 = ./error_500.html
```

### Configuration Options

| Option | Description | Default |
|--------|-------------|--------|
| `port` | Port number to listen on | 6969 |
| `root_dir` | Root directory for serving files | `.` (current directory) |
| `error_404` | Path to custom 404 error page | `./error_404.html` |
| `error_500` | Path to custom 500 error page | `./error_500.html` |

## Building

```bash
# Clean and build
make clean && make

# Build only
make
```

## Project Structure

```
http/
├── src/
│   ├── http_server.c    # Main server implementation
│   ├── config.c         # Configuration parser
│   └── config.h         # Configuration header
├── build/
│   └── http_server      # Compiled binary
├── index.html           # Default homepage
├── error_404.html       # Custom 404 page
├── error_500.html       # Custom 500 page
├── Makefile             # Build configuration
└── README.md            # This file
```

## License

MIT
