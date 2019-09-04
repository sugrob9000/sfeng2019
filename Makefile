EXEC = bin/engine

CC = g++
CFLAGS = -O1 -Wno-attributes

FILES = \
	src/main.cpp \
	src/render.cpp \


ifeq ($(OS), Windows_NT)

# Windows

LIBS = -lmingw32 -llibSDL2 -lOpenGL32
EXEC := $(EXEC).exe
FILES += include/GL/glew.c

CFLAGS += -DWINDOWS -Iinclude
ifeq ($(LBITS), 64)
CFLAGS += -Llib/64
else
CFLAGS += -Llib/86
endif

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
	$(CC) $^ $(CFLAGS) $(LIBS) -o $@

run: all
	$(EXEC)

