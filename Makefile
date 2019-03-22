
TARGET   := prog
SRCDIR   := .
ZFLOG_DIR := zf_log
BINDIR   := bin

WARNINGS := -Wall -Wextra
INC_DIRS := -I. -Izf_log

CC       := gcc
CFLAGS   := -std=c99 $(WARNINGS) $(INC_DIRS)

LINKER   := gcc
LFLAGS   := $(WARNINGS) $(INC_DIRS)

SOURCES  := $(wildcard $(SRCDIR)/*.c)
ZFLOG_SRC := $(wildcard $(ZFLOG_DIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(patsubst $(SRCDIR)/%.c,$(BINDIR)/%.o,$(SOURCES))
ZFLOG_OBJ := $(patsubst $(ZFLOG_DIR)/%.c,$(BINDIR)/%.o,$(ZFLOG_SRC))

.PHONY: all clean compile
default: all

all: $(BINDIR)/$(TARGET)

compile: $(OBJECTS) $(ZFLOG_OBJ)

clean:
	rm -f $(OBJECTS) $(ZFLOG_OBJ) $(BINDIR)/$(TARGET)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/$(TARGET): $(OBJECTS) $(ZFLOG_OBJ) $(BINDIR)
	$(LINKER) $(OBJECTS) $(ZFLOG_OBJ) $(LFLAGS) -o $@

$(OBJECTS): $(BINDIR)/%.o: $(SRCDIR)/%.c $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(ZFLOG_OBJ): $(BINDIR)/%.o: $(ZFLOG_DIR)/%.c $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
