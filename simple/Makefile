CC := gcc
OBJ := hello.o bye.o
CFLAGS := 

all: main

%.o: %.c
	@echo cc + $<
	@$(CC) $(CFLAGS) -c -o $@ $<

main: $(OBJ)
	@echo ld + $(OBJ)
	@$(CC) -o $@ $(OBJ)

run: main
	@./main world

clean:
	rm $(OBJ) main