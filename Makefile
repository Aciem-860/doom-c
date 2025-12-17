CC = cc
CFLAGS = -g -O3 -Iinclude -Ilogging-c -Wall -Wpedantic
LDFLAGS = -L/usr/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lm
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/doom

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(subst $(SRC_DIR), $(BUILD_DIR), $(patsubst %.c,%.o,$(SRC)))

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@if [ ! -d "$(BUILD_DIR)" ]; then mkdir -p $(BUILD_DIR); fi
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -fr $(OBJ) $(TARGET)
	rm -fr $(BUILD_DIR)
