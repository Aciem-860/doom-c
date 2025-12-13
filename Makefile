CC = cc
CFLAGS = -g -O3 -Iinclude -Ilogging-c
LDFLAGS = -L/usr/lib -lSDL2 -lSDL2_image -lm
TARGET = doom

SRC = src/main.c src/sector.c src/player.c src/vector.c src/context.c
OBJ = $(patsubst %.c,%.o,$(SRC))

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -fr $(OBJ) $(TARGET)
