all: myftp myftpserve

myftp: myftp.o
	gcc -o myftp myftp.o

myftp.o: myftp.c
	gcc -c myftp.c

myftpserve: myftpserve.o
	gcc -o myftpserve myftpserve.o

myftpserve.o: myftpserve.c
	gcc -c myftpserve.c

clean:
	rm *.o myftp myftpserve
