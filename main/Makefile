CC = gcc
CFLAGS = -Wall -Wextra -O2
LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lX11

TARGET = tamagotchi
SRC = main.c game.c draw.c
HDR = game.h draw.h

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
