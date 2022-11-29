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
#include<sys/wait.h>
#include<netdb.h>
#include"myftp.h"

#define BACKLOG 4

int setupPort(struct addrinfo *this){
    int listenfd;

    // Make a reuseable socket, bind it to a port, and establish as a server 
    if((listenfd = socket(this->ai_family, this->ai_socktype, 0)) == -1 || setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1 || bind(listenfd, this->ai_addr, this->ai_addrlen) == -1 || listen(listenfd, BACKLOG) == -1){
        fprintf(stderr, "Error: %s\n", strerror(errno));
        freeaddrinfo(this);
        exit(1);
    }

    freeaddrinfo(this);
    return listenfd;
}

void main(int argc, char *argv[]){

    char host[NI_MAXHOST], buffer[BUFFER_SIZE] = {0}, *exitSend = "Closing Server Connection\0";
    int connectfd, num;
    struct sockaddr_storage clientAddr;
    socklen_t addrlen = sizeof(struct sockaddr_storage); pid_t here;

    struct addrinfo *this = getAddr(NULL, "49999");
    int listenfd = setupPort(this);

    for(;;){
        if((connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &addrlen)) == -1){ // Accept connection
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(1);
        }
        
        if((here = fork()) == -1){ // Fork
            fprintf(stderr, "Error: %s\n", strerror(errno));
            close(connectfd);
            exit(1);
        }

        else if(here){ // Parent
            close(connectfd);

            if(waitpid(-1, NULL, WNOHANG) == -1){ // Clean up child
                fprintf(stderr, "Error: %s\n", strerror(errno));
                exit(1);
            }
        }

        else{ // Child
            close(listenfd);

            int errNum;
            if((errNum = getnameinfo((struct sockaddr *) &clientAddr, addrlen, host, NI_MAXHOST, NULL, 0, NI_NUMERICSERV)) != 0){ // Convert socket addr to host name
                fprintf(stderr, "Error: %s\n", gai_strerror(errNum));
                exit(1);
            }

            fprintf(stdout, "Control Connection Established from: %s\n", host);

            for(;;){
                num = read(connectfd, buffer, BUFFER_SIZE); // Write to client and close connection

                if(!strncmp(buffer, "Q\0", num)){ // Exit recieved
                    write(connectfd, exitSend, 27);
                    fprintf(stdout, "Q : Success\n");
                    exit(0);
                }

                fprintf(stdout, "%s\n", buffer);
            }
        }
    }
}
