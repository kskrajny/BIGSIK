CC = gcc
CFLAGS = -Wall
TARGETS = testhttp_raw testhttp
.PHONY : clean

all: $(TARGETS) 

err.o: err.c err.h

testhttp.o: testhttp.c err.h

testhttp: testhttp.o err.o

testhttp_raw.o: testhttp_raw.c err.h

testhttp_raw: testhttp_raw.o err.o

clean:
	rm -f *.o *~ $(TARGETS) 
