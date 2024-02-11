CC = gcc
INCDIR = ./include

SRC1 = cimin.c
OBJ1 = $(SRC1:.c=.o)
EXE1 = cimin

# rule for link
all: $(EXE1)
$(EXE1): $(OBJ1)
	$(CC) -o $@ $^

# rule for compilation
%.o: %.c
	$(CC) -c -I$(INCDIR) $<

.PHONY: all clean
clean:
	rm -f *.o *.exe $(EXE1)