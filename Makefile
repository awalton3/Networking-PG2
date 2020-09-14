CFLAGS=-g -c -std=c++11
LDFLAGS=-g -lcrypto -lz
 
all: myftp myftpd

myftp: myftp.o
	g++ $(LDFLAGS) -o myftp myftp.o

myftpd: myftpd.o
	g++ $(LDFLAGS) -o myftpd myftpd.o

myftp.o: myftp.cpp
	g++ $(CFLAGS) myftp.cpp

myftpd.o: myftpd.cpp
	g++ $(CFLAGS) myftpd.cpp

clean: 
	rm -f *.o myftp myftpd


