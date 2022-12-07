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

/*Complete commands
    ProfServ + MyClient     |     MyServ + ProfClient     |     MyClient + MyClient
    exit ls cd rls rcd            exit ls cd rls rcd            exit ls cd rls rcd
    show get                      show get                      show get
*/
void setCommands(char *commands[]){
    commands[0] = "exit"; commands[1] = "cd"; commands[2] = "rcd"; commands[3] = "ls";
    commands[4] = "rls"; commands[5] = "get"; commands[6] = "show"; commands[7] = "put";
}

int analyizeInput(const char *str){ // Returns array index of commands
    char *commands[8], token[BUFFER_SIZE]; setCommands(commands);
    sscanf(str, "%s", token);
    int i;
    for(i = 0; i < 8; i++) if(!strcmp(token, commands[i])) break;
    if(i == 0 || i == 3 || i == 4) if(strcmp(str, commands[i])) i = 8; // exit | ls | rls
    return i;
}

void localCD(char *path){
    struct stat area, *s = &area;
    if(access(path, R_OK) != 0 || lstat(path, s) != 0) fprintf(stderr, "%s\n", strerror(errno));
    else if(S_ISDIR(s->st_mode)){
        if(chdir(path) != 0) fprintf(stderr, "%s\n", strerror(errno));
    }
    else fprintf(stderr, "No such directory\n");
}

void makeFile(char *filename, int fd) {
    FILE *file = fopen(filename, "w"); // Open file for writing
    if (file == NULL) fprintf(stderr, "Error: Could not create file '%s'\n", filename);
    else {
        fprintf(stdout, "Success: File '%s' created\n", filename);
        char buf[BUFFER_SIZE];
        int n;
        while ((n = read(fd, buf, BUFFER_SIZE)) > 0) fwrite(buf, 1, n, file);
    }
    fclose(file);
}

void exeCommand(int i, int fd, char *str){
    char buffer[BUFFER_SIZE]; char *path;
    int connectfd;
    struct addrinfo *data;
    pid_t p;
    switch(i){
        case 0: // Exit
            write(fd, "Q\n", 2);
            readParseAndLog(fd, buffer, 1, 1);
            exit(0);
        case 1: // CD
            strtok(str, " ");
            path = strtok(NULL, "\0");
            localCD(path);
            break;
        case 2: // RCD
            strtok(str, " ");
            path = strtok(NULL, "\0");
            write(fd, buffer, sprintf(buffer, "C%s\n", path));
            readParseAndLog(fd, buffer, 1, 1);
            break;
        case 3: // LS
            pid_t gp;
            if(gp = fork()){ // Parent
                waitpid(gp, NULL, 0);
            } else { // Child
                int fd[2];
                if(pipe(fd) != 0){ fprintf(stderr, "Error: %s\n", strerror(errno)); exit(1); }

                if(p = fork()){ // Parent
                    if(close(fd[1]) != 0){ fprintf(stderr, "%s\n", strerror(errno)); exit(1); }
                    if(dup2(fd[0], 0) == -1){ fprintf(stderr, "%s\n", strerror(errno)); exit(1); } // Pipe reads from STDIN
                    if(close(fd[0]) != 0){ fprintf(stderr, "%s\n", strerror(errno)); exit(1); }

                    waitpid(p, NULL, 0);
                    execlp("more", "more", "-20", NULL);
                } else { // Child
                    if(close(fd[0]) != 0){ fprintf(stderr, "%s\n", strerror(errno)); exit(1); }
                    if(dup2(fd[1], 1) == -1){ fprintf(stderr, "%s\n", strerror(errno)); exit(1); } // Pipe writes to STDOUT
                    if(close(fd[1]) != 0){ fprintf(stderr, "%s\n", strerror(errno)); exit(1); }
                    
                    execlp("ls", "ls", "-l", NULL);
                }
            }
            break;
        case 4: // RLS
            write(fd, "D\n", 2);
            if(readParseAndLog(fd, buffer, 1, 1)) break;
            sscanf(buffer, "A%s", buffer);
            data = getAddr(NULL, buffer);
            connectfd = connectToPort(data);
            write(fd, "L\n", 2);
            if(readParseAndLog(fd, buffer, 1, 1)){
                close(connectfd);
                break;
            }

            if(p = fork()){ // Parent
                waitpid(p, NULL, 0);
            } else { // Child
                if(dup2(connectfd, 0) == -1){ fprintf(stderr, "%s\n", strerror(errno)); exit(1); } // Writes to STDIN
                if(close(connectfd) != 0){ fprintf(stderr, "%s\n", strerror(errno)); exit(1); }
                execlp("more", "more", "-20", NULL);
            }

            close(connectfd);
            break;
        case 5: // Get
            strtok(str, " ");
            path = strtok(NULL, "\0");

            if (access(path, F_OK) != -1) {
                printf("Error: File '%s' already exists\n", path);
                break;
            }

            write(fd, "D\n", 2);
            if(readParseAndLog(fd, buffer, 1, 1)) break;
            sscanf(buffer, "A%s", buffer);
            data = getAddr(NULL, buffer);
            connectfd = connectToPort(data);

            write(fd, buffer, sprintf(buffer, "G%s\n", path));
            if(readParseAndLog(fd, buffer, 0, 1)){
                close(connectfd);
                break;
            }

            makeFile(path, connectfd);
            close(connectfd);
            break;
        case 6: // Show
            write(fd, "D\n", 2);
            if(readParseAndLog(fd, buffer, 1, 1)) break;
            sscanf(buffer, "A%s", buffer);
            data = getAddr(NULL, buffer);
            connectfd = connectToPort(data);

            strtok(str, " ");
            path = strtok(NULL, "\0");
            write(fd, buffer, sprintf(buffer, "G%s\n", path));
            if(readParseAndLog(fd, buffer, 1, 1)){
                close(connectfd);
                break;
            }

            if(p = fork()){ // Parent
                waitpid(p, NULL, 0);
            } else { // Child
                if(dup2(connectfd, 0) == -1){ fprintf(stderr, "%s\n", strerror(errno)); exit(1); } // Writes to STDIN
                if(close(connectfd) != 0){ fprintf(stderr, "%s\n", strerror(errno)); exit(1); }
                execlp("more", "more", "-20", NULL);
            }

            close(connectfd);
            break;
        case 7: // Put
            write(fd, "D\n", 2);
            if(readParseAndLog(fd, buffer, 1, 1)) break;
            sscanf(buffer, "A%s", buffer);
            data = getAddr(NULL, buffer);
            connectfd = connectToPort(data);

            strtok(str, " ");
            path = strtok(NULL, "\0");
            write(fd, buffer, sprintf(buffer, "P%s\n", path));
            if(readParseAndLog(fd, buffer, 1, 1)){
                close(connectfd);
                break;
            }

            break;
        default: 
            fprintf(stderr, "Error: Invalid Command\n");
    }
}

void main(int argc, char *argv[]){
    char buffer[BUFFER_SIZE];

    struct addrinfo *this = getAddr(argv[1], "49999");
    int connectfd = connectToPort(this);

    for(;;){
        fprintf(stdout, "Command: "); fflush(NULL);
        readParseAndLog(0, buffer, 0, 0);
        exeCommand(analyizeInput(buffer), connectfd, buffer);
    }
}