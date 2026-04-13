CC = gcc
CFLAGS = -Wall -Wextra -O2
SRCDIR = src
BUILDDIR = build
TARGET = $(BUILDDIR)/http_server

all: $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(TARGET): $(SRCDIR)/http_server.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCDIR)/http_server.c

clean:
	rm -rf $(BUILDDIR)/*

.PHONY: all clean
