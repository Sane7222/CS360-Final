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
#include"myftp.h"

void removeTrailingWhiteSpace(char *str){
    int index = 0, i = 0;
    while(str[i] != '\0') str[i] != ' ' && str[i] != '\t' ? index = ++i : i++;
    str[index] = '\0';
}

void main(int argc, char *argv[]){

    char buffer[BUFFER_SIZE] = {0}, *exitCond = "exit\0";
    int num;

    struct addrinfo *this = getAddr(argv[1], "49999");
    int connectfd = connectAsClient(this);
    
    fprintf(stdout, "Connection successful\n");
    
    for(;;){
        fprintf(stdout, "Please enter command:\n");
        num = read(0, buffer, BUFFER_SIZE);
        buffer[num-1] = '\0';
        sscanf(buffer, " %[^\n]", buffer);
        removeTrailingWhiteSpace(buffer);

        if(!strncmp(buffer, exitCond, num)){ // Exit recieved
            write(connectfd, "Q\0", 2);
            read(connectfd, buffer, BUFFER_SIZE);
            fprintf(stdout, "%s\n", buffer);
            exit(0);
        }

        write(connectfd, buffer, num); // Write command to server
    }
}