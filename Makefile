CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image -lm

# Detect OS for SDL configuration
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CFLAGS += $(shell sdl2-config --cflags)
else ifeq ($(UNAME_S),Linux)
	CFLAGS += $(shell sdl2-config --cflags)
else
	# Windows - assumes SDL2 path is set or in the include path
	CFLAGS += -Iinclude
	LDFLAGS = -lSDL2 -lSDL2_image -lm
endif

SRC = main.c engine.c
OBJ = $(SRC:.c=.o)
TARGET = raycaster

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean 