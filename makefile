#Created by: Calvin Nichols 0588016

CC = gcc
EXEC1 = dealer
EXEC2 = blackjackd
OBJ1 = dealer.o
OBJ2 = blackjackd.o
OBJ3 = DeckImplementation.o
OBJ4 = TimerListImplementation.o

all: $(EXEC1) $(EXEC2)
blackjack: $(EXEC1) $(EXEC2)

$(EXEC1): $(OBJ1) $(OBJ3) $(OBJ4)
	$(CC) $(OBJ1) $(OBJ3) $(OBJ4) -o $(EXEC1)

$(EXEC2): $(OBJ2) $(OBJ3) $(OBJ4)
	$(CC) $(OBJ2) $(OBJ3) $(OBJ4) -o $(EXEC2)

dealer.o: dealer.c DeckInterface.h TimerListInterface.h
blackjackd.o: blackjackd.c DeckInterface.h TimerListInterface.h
DeckImplementation.o: DeckImplementation.c DeckInterface.h
TimerListImplementation.o: TimerListImplementation.c TimerListInterface.h

clean:
	@ rm *.o

