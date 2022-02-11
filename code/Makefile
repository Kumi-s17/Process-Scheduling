CC = gcc
CFLAGS = -Wall -o -g
LIB = -lm
HDR = list.h process.h processor.h statistics.h optimization.h

SRC1 = allocate.c list.c process.c processor.c statistics.c optimization.c
OBJ1 = $(SRC1:.c=.o)
EXE1 = allocate

all: $(EXE1) 
$(EXE1): $(HDR) $(OBJ1)
	$(CC) $(CFLAGS) -o $(EXE1) $(OBJ1) $(LIB)


clean:
	rm -f $(EXE1) *.o

$(OBJ1): $(HDR)