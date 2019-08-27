EXEC-NAME = engine

CC = g++
CFLAGS = -O2 -o $(EXEC-NAME).out

LIBS = -lSDL2 -lGL -lGLEW

linux:
	$(CC) src/main.cpp $(CFLAGS) -DLINUX $(LIBS)

windows:

runl: linux
	./$(EXEC-NAME).out
runw: windows
	$(EXEC-NAME).exe
