CC = cc
CFLAGS = -g -O3 -Iinclude -Ilogging-c
LDFLAGS = -L/usr/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lm
TARGET = doom

SRC = $(wildcard src/*.c)
OBJ = $(patsubst %.c,%.o,$(SRC))

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -fr $(OBJ) $(TARGET)
