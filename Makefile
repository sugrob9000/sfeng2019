EXEC = bin/$(LBITS)/engine

CC = g++
CFLAGS = -O1 -Wno-attributes -Isrc

FILES-CPP = $(shell find src/ -name "*.cpp")
FILES-H = $(shell find src/ -name "*.h")

LBITS = $(shell getconf LONG_BIT)

ifeq ($(OS), Windows_NT)

# Windows

LIBS = -lmingw32 -lSDL2 -lOpenGL32
EXEC := $(EXEC).exe
FILES-CPP += include/GL/glew.c

CFLAGS += -DWINDOWS -Iinclude
CFLAGS += -Llib/$(LBITS)

else

# Linux

LIBS = -lSDL2 -lGL -lGLEW
EXEC := $(EXEC).out
CFLAGS += -DLINUX

endif

all: $(EXEC)
	@echo Up to date

$(EXEC): $(FILES-CPP) $(FILES-H)
	@echo Rebuilding
	$(CC) $(FILES-CPP) $(CFLAGS) $(LIBS) -o $@

run: all
	$(EXEC)

