CFLAGS=-g -c -std=c++11
LDFLAGS=-g -lcrypto -lz
 
all: myftpd

myftpd: myftpd.o
	g++ $(LDFLAGS) -o myftpd myftpd.o

myftpd.o: myftpd.cpp
	g++ $(CFLAGS) myftpd.cpp

clean: 
	rm -f *.o myftpd


