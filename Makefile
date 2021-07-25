CCFLAGS     =  -Wall -pedantic -g

PROG        = 2048


OBJS        = 	io.o \
			  	printer.o \
				board.o \
				game.o



all : $(PROG)

$(PROG) : main.c $(OBJS)
	$(CC) $(CCFLAGS) -o $(PROG) main.c $(OBJS)

%.o : %.c
	$(CC) $(CCFLAGS) -c $< -o $@

depend:
	$(CC) *.c -MM > .depend

include .depend

clean:
	rm -f $(OBJS) $(PROG)
