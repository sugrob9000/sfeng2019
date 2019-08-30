EXEC = engine

CC = g++
CFLAGS = -O1 -Wno-attributes -o bin/$(EXEC)

FILES = \
	src/main.cpp


ifeq ($(OS), Windows_NT)
# Windows

LIBS = -lmingw32 -lSDL2 -lOpenGL32
EXEC := $(EXEC).exe
FILES += include/GL/glew.c
CFLAGS += -DWINDOWS -Iinclude -Llib

else
# Linux

LIBS = -lSDL2 -lGL -lGLEW
EXEC := $(EXEC).out
CFLAGS += -DLINUX

endif

all: $(EXEC)
	@echo Up to date

$(EXEC): $(FILES)
	@echo Rebuilding
	$(CC) $(FILES) $(CFLAGS) $(LIBS)

run: all
	bin/$(EXEC)
