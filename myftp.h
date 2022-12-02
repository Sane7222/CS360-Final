#ifndef MYFTP_H
#define MYFTP_H

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<netdb.h>

#define BUFFER_SIZE 512

struct addrinfo * getAddr(char *name, char *portNum){
    int errNum;
    struct addrinfo addr = {0}, *this;

    addr.ai_family = AF_INET;
    addr.ai_socktype = SOCK_STREAM;

    if((errNum = getaddrinfo(name, portNum, &addr, &this)) != 0){
        fprintf(stderr, "Error: %s\n", gai_strerror(errNum));
        freeaddrinfo(this);
        exit(1);
    }

    return this;
}

#endif