EXEC-NAME = engine

CC = g++
CFLAGS = -O2

LIBS = -lSDL2 -lGL -lGLEW

FILES = src/main.cpp \


linux:
	$(CC) $(FILES) $(CFLAGS) $(LIBS) -o $(EXEC-NAME).out -DLINUX

windows:
	$(CC) $(FILES) $(CFLAGS) -o $(EXEC-NAME).exe -DWINDOWS

runl: linux
	./$(EXEC-NAME).out
runw: windows
	$(EXEC-NAME).out
