CC = gcc
CFLAGS = -Wall -Wextra -O2
SRCDIR = src
BUILDDIR = build
TARGET = $(BUILDDIR)/http_server
SRCS = $(SRCDIR)/http_server.c $(SRCDIR)/config.c

all: $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(TARGET): $(SRCS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -rf $(BUILDDIR)/*

install-config:
	@echo "Creating config directory at ~/.config/servec/"
	@mkdir -p ~/.config/servec
	@if [ ! -f ~/.config/servec/config ]; then \
		echo "# servec configuration file" > ~/.config/servec/config; \
		echo "# port = 8080" >> ~/.config/servec/config; \
		echo "# root_dir = ." >> ~/.config/servec/config; \
		echo "# error_404 = /path/to/404.html" >> ~/.config/servec/config; \
		echo "# error_500 = /path/to/500.html" >> ~/.config/servec/config; \
		echo "Config file created. Edit ~/.config/servec/config to customize."; \
	else \
		echo "Config file already exists at ~/.config/servec/config"; \
	fi

.PHONY: all clean install-config
