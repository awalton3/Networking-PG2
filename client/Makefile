CFLAGS=-g -c -std=c++11
LDFLAGS=-g -lcrypto -lz
 
all: myftp

myftp: myftp.o
	g++ $(LDFLAGS) -o myftp myftp.o

myftp.o: myftp.cpp
	g++ $(CFLAGS) myftp.cpp

clean: 
	rm -f *.o myftp


