EXEC-NAME = engine

CC = g++
CFLAGS = -O2

LIBS-LINUX = -lSDL2 -lGL -lGLEW
LIBS-WINDOWS = -lmingw32 -lSDL2 -lOpenGL32

FILES = src/main.cpp \


linux:
	$(CC) $(FILES) $(CFLAGS) $(LIBS-LINUX) -o bin/$(EXEC-NAME).out -DLINUX

win:
	$(CC) $(FILES) $(CFLAGS) $(LIBS-WINDOWS) -o bin/$(EXEC-NAME).exe -DWINDOWS -Iinclude -Llib

runl: linux
	bin/$(EXEC-NAME).out
runw: win
	bin/$(EXEC-NAME).exe
