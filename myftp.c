/*     Matias Moseley     11/27/2022     CS 360     Final     */

#include"myftp.h"

int connectToPort(struct addrinfo *this){
    int connectfd;

    if((connectfd = socket(this->ai_family, this->ai_socktype, 0)) == -1 || connect(connectfd, this->ai_addr, this->ai_addrlen) == -1){ // Create a socket and connect it to server port
        fprintf(stderr, "Error: %s\n", strerror(errno));
        freeaddrinfo(this);
        exit(1);
    }

    freeaddrinfo(this);
    return connectfd;
}

int removeTrailingWhiteSpace(char *str){ // Returns length of string
    int index = 0, i = 0;
    while(str[i] != '\0') str[i] != ' ' && str[i] != '\t' ? index = ++i : i++;
    str[index] = '\0';
    return i;
}

void parseAndLog(int fd, char *buffer, int logOpt){
    int num = read(fd, buffer, BUFFER_SIZE);
    buffer[num-1] = '\0';
    sscanf(buffer, " %[^\n]", buffer);
    removeTrailingWhiteSpace(buffer);
    if(logOpt) fprintf(stdout, "%s\n", buffer);
}

void setCommands(char *commands[]){
    commands[0] = "exit"; commands[1] = "cd"; commands[2] = "rcd"; commands[3] = "ls";
    commands[4] = "rls"; commands[5] = "get"; commands[6] = "show"; commands[7] = "put";
}

int analyizeInput(const char *str){ // Returns array index of commands
    char *commands[8], token[BUFFER_SIZE]; setCommands(commands);
    sscanf(str, "%s", token);
    int i;
    for(i = 0; i < 8; i++) if(!strcmp(token, commands[i])) break;
    return i;
}

void localCD(char *path){
    struct stat area, *s = &area;
    if(access(path, R_OK) != 0 || lstat(path, s) != 0) fprintf(stderr, "Error: %s\n", strerror(errno));
    else if(S_ISDIR(s->st_mode)){
        if(chdir(path) != 0) fprintf(stderr, "Error: %s\n", strerror(errno));
    }
    else fprintf(stderr, "Error: Not a Directory <%s>\n", path);
}

void exeCommand(int i, int fd, char *str){
    char buffer[BUFFER_SIZE];
    switch(i){
        case 0: // Exit
            write(fd, "Q", 2);
            parseAndLog(fd, buffer, 1);
            exit(0);
        case 1: // CD
            strtok(str, " ");
            char *path = strtok(NULL, " ");
            localCD(path);
            break;
        case 2: // RCD
            write(fd, "C", 2);
            parseAndLog(fd, buffer, 1);
            break;
        case 3: // LS
            break;
        case 4: // RLS
            write(fd, "D", 2);
            parseAndLog(fd, buffer, 1);
            write(fd, "L", 2);
            parseAndLog(fd, buffer, 1);
            break;
        case 5: // Get
        case 6: // Show
            write(fd, "D", 2);
            parseAndLog(fd, buffer, 1);
            write(fd, "G", 2);
            parseAndLog(fd, buffer, 1);
            break;
        case 7: // Put
            write(fd, "D", 2);
            parseAndLog(fd, buffer, 1);
            write(fd, "P", 2);
            parseAndLog(fd, buffer, 1);
            break;
        default: 
            fprintf(stdout, "Error: Invalid Command\n");
    }
}

void main(int argc, char *argv[]){
    char buffer[BUFFER_SIZE];

    struct addrinfo *this = getAddr(argv[1], "49999");
    int connectfd = connectToPort(this);

    for(;;){
        fprintf(stdout, "Command: "); fflush(NULL);
        parseAndLog(0, buffer, 0);
        exeCommand(analyizeInput(buffer), connectfd, buffer);
    }
}