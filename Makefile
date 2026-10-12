# Compiler flags to find header files in include/
CFLAGS := -Iinclude

# Object files generated from all *.c sources
OBJ := $(patsubst %.c, %.o, $(wildcard *.c src/*.c))

# Build the executable
mp3tag: $(OBJ)
	@gcc $(CFLAGS) -o $@ $^

# Clean up generated files
clean:
	@rm -f *.exe *.o src/*.o mp3tag
