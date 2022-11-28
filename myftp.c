/*
 *    Matias Moseley     11/27/2022     CS 360     Final
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>

#define PORT_NUM "49999"
#define BACKLOG 1
#define BUFFER_SIZE 512

void removeTrailingWhiteSpace(char *str){
    int index = 0, i = 0;
    while(str[i] != '\0') str[i] != ' ' && str[i] != '\t' ? index = ++i : i++;
    str[index] = '\0';
}

void main(int argc, char *argv[]){

    char buffer[BUFFER_SIZE] = {0}, *exitCond = "exit\0";
    int connectfd, errNum, num;
    struct addrinfo cliAddr = {0}, *this;

    cliAddr.ai_family = AF_INET;
    cliAddr.ai_socktype = SOCK_STREAM;

    if((errNum = getaddrinfo(argv[1], PORT_NUM, &cliAddr, &this)) != 0){ // Get internet addr of argv[1]
        fprintf(stderr, "Error: %s\n", gai_strerror(errNum));
        freeaddrinfo(this);
        exit(1);
    }

    if((connectfd = socket(this->ai_family, this->ai_socktype, 0)) == -1 || connect(connectfd, this->ai_addr, this->ai_addrlen) == -1){ // Create a socket and connect it to server port
        fprintf(stderr, "Error: %s\n", strerror(errno));
        freeaddrinfo(this);
        exit(1);
    }

    freeaddrinfo(this);
    
    for(;;){
        fprintf(stdout, "Please enter command:\n");
        num = read(0, buffer, BUFFER_SIZE);
        buffer[num-1] = '\0';
        sscanf(buffer, " %[^\n]", buffer);
        removeTrailingWhiteSpace(buffer);

        write(connectfd, buffer, num); // Write command to server
        if(!strncmp(buffer, exitCond, num)) exit(0); // Exit recieved
    }
}