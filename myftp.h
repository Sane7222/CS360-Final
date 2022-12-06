#ifndef MYFTP_H
#define MYFTP_H

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
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

int removeTrailingWhiteSpace(char *str){ // Returns length of string
    int index = 0, i = 0;
    while(str[i] != '\0') str[i] != ' ' && str[i] != '\t' ? index = ++i : i++;
    str[index] = '\0';
    return i;
}

int readParseAndLog(int fd, char *buffer, int logOpt, int fromServ){
    char byte[1];
    int i = 0;
    while(read(fd, byte, 1) != 0 && byte[0] != '\n'){
        buffer[i++] = byte[0];
    }
    buffer[i] = '\0';
    sscanf(buffer, " %[^\n]", buffer);
    removeTrailingWhiteSpace(buffer);
    if(logOpt) fprintf(stdout, "%s\n", buffer);
    if(fromServ){
        if((char)buffer[0] == 'E'){
            sscanf(buffer, "E%[^\n]", buffer);
            fprintf(stderr, "%s\n", buffer);
            return 1;
        }
        else return 0;
    }
    return 0;
}

#endif