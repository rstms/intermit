
OBJS =	intermit.o

ALL: intermit

intermit.o: intermit.c

intermit: ${OBJS}
	gcc -o intermit ${OBJS}

install:
	strip intermit
	cp intermit /usr/local/bin

clean:
	rm -f *.o *.c~ intermit
