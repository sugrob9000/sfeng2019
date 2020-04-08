BIN = bin/$(LBITS)
SRC = src

EXEC = $(BIN)/engine

WARNINGS = \
	-Wall \
	-Wextra \
	-Wno-sign-compare \
	-Wno-unused \
	-Wno-attributes \
	-Wno-cast-function-type \
	-Wshadow
ENGINE-FLAGS =

CC = g++
CFLAGS = -Og -g $(WARNINGS) --std=gnu++17 -Isrc -Iinclude -fmax-errors=1

FILES-CPP = $(shell find src/ -type f -name "*.cpp")
FILES-O = $(FILES-CPP:$(SRC)/%.cpp=$(BIN)/%.o)

LBITS = $(shell getconf LONG_BIT)

LIBS = -lSDL2_image -lSDL2_ttf

ifeq ($(OS), Windows_NT)
# Windows

CFLAGS += -DWINDOWS
LIBS += -Llib/$(LBITS)
LIBS += -lSDL2 -lopengl32 -lmingw32 -lGLU32

EXEC := $(EXEC).exe
FILES-O += $(BIN)/glew.o

else
# Linux

LIBS += -lSDL2 -lGL -lGLEW -lGLU
EXEC := $(EXEC).out
CFLAGS += -DLINUX

endif

all: commands $(EXEC)
	@echo $(EXEC) is up to date

commands:
	@./regen_cmds.sh $(FILES-CPP)

$(EXEC): $(FILES-O)
	@echo Linking $@
	@$(CC) $^ $(LIBS) -o $@

$(BIN)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	@echo Compiling $@
	@$(CC) -MMD -c $(CFLAGS) $< -o $@

	@cp $(BIN)/$*.d $(BIN)/$*.P;
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	     -e '/^$$/ d' -e 's/$$/ :/' < $(BIN)/$*.d >> $(BIN)/$*.P; \
	rm -f $(BIN)/$*.d
-include $(BIN)/*.P

$(BIN)/glew.o: include/GL/glew.c
	@echo Compiling $@
	@$(CC) -c $(CFLAGS) $^ -o $@

run: all
	@echo Running:
	$(EXEC) $(ENGINE-FLAGS)
