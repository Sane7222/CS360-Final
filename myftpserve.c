/*
 *    Matias Moseley     11/17/2022     CS 360
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
#include<time.h>

#define PORT_NUM "49999"
#define BACKLOG 1

void main(int argc, char *argv[]){

    char host[NI_MAXHOST], buf[32] = {0};
    int listenfd, connectfd, errNum, log = 0;
    struct addrinfo servAddr = {0}, *this;
    struct sockaddr_storage clientAddr;
    time_t date; socklen_t addrlen; pid_t here;

    servAddr.ai_family = AF_INET;
    servAddr.ai_socktype = SOCK_STREAM;

    if((errNum = getaddrinfo(NULL, PORT_NUM, &servAddr, &this)) != 0){ // Get internet addr
        fprintf(stderr, "Error: %s\n", gai_strerror(errNum));
        freeaddrinfo(this);
        exit(1); }

    // Make a reuseable socket, bind it to a port, and establish as a server 
    if((listenfd = socket(this->ai_family, this->ai_socktype, 0)) == -1 || setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1 || bind(listenfd, this->ai_addr, this->ai_addrlen) == -1 || listen(listenfd, BACKLOG) == -1){
        fprintf(stderr, "Error: %s\n", strerror(errno));
        freeaddrinfo(this);
        exit(1); }

    freeaddrinfo(this);

    for(;;){
        addrlen = sizeof(struct sockaddr_storage);
        if((connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &addrlen)) == -1){ // Accept connection
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(1); }

        log++;
        if((here = fork()) == -1){ // Fork
            fprintf(stderr, "Error: %s\n", strerror(errno));
            close(connectfd);
            exit(1); }

        else if(here){ // Parent
            close(connectfd);

            if(waitpid(-1, NULL, WNOHANG) == -1){ // Clean up child
                fprintf(stderr, "Error: %s\n", strerror(errno));
                exit(1); }
        }

        else{ // Child
            close(listenfd);

            if((errNum = getnameinfo((struct sockaddr *) &clientAddr, addrlen, host, NI_MAXHOST, NULL, 0, NI_NUMERICSERV)) != 0){ // Convert socket addr to host name
                fprintf(stderr, "Error: %s\n", gai_strerror(errNum));
                exit(1); }

            fprintf(stdout, "%s %d\n", host, log);

            date = time(NULL);
            snprintf(buf, sizeof(buf), "%.18s\n", ctime(&date)); // Date and time

            write(connectfd, buf, sizeof(buf)); // Write to client and close connection
            exit(0); }
    }
}
