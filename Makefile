CUR_DIR := $(shell pwd)
SRC_FILES := linenoise.c
OBJ_FILES := $(patsubst %.c, %.o, $(SRC_FILES))

VERSION_MAJOR := 1
VERSION_MINOR := 0
VERSION := $(VERSION_MAJOR).$(VERSION_MINOR)
LIBNAME := linenoise
PKG_NAME := lib$(LIBNAME)-$(VERSION)

CC := gcc
AR := ar
CFLAGS := -c -fPIC -g -Wall
LDFLAGS := -s -shared -fvisibility=hidden -Wl,--exclude-libs=ALL,--no-as-needed,-soname,lib$(LIBNAME).so.$(VERSION_MAJOR)
PREFIX ?= /usr

.PHONY: all
all: linenoise

.PHONY: linenoise
linenoise: $(SRC_FILES) $(OBJ_FILES)
	@echo "Building $(PKG_NAME)..."
	$(CC) $(LDFLAGS) $(OBJ_FILES) -o lib$(LIBNAME).so.$(VERSION_MAJOR)
	$(AR) rcs lib$(LIBNAME).a $(OBJ_FILES)
	
.PHONY: install
install: all
	install --directory $(PREFIX)/lib $(PREFIX)/include
	install lib$(LIBNAME).so.$(VERSION_MAJOR) lib$(LIBNAME).a $(PREFIX)/lib/
	ln -fs $(PREFIX)/lib/lib$(LIBNAME).so.$(VERSION_MAJOR) $(PREFIX)/lib/lib$(LIBNAME).so
	install yuarel.h $(PREFIX)/include/
	ldconfig -n $(PREFIX)/lib

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

linenoise_example: linenoise.h linenoise.c

linenoise_example: linenoise.c example.c
	$(CC) -Wall -W -Os -g -o linenoise_example linenoise.c example.c

.PHONY: clean
clean:
	rm -f *.o
	rm -f linenoise_example
	
