/*     Matias Moseley     11/27/2022     CS 360     Final     */

#include"myftp.h"

#define BACKLOG 4

int setupPort(struct addrinfo *this, char *ascii){
    int listenfd;

    // Make a reuseable socket, bind it to a port, and establish as a server 
    if((listenfd = socket(this->ai_family, this->ai_socktype, 0)) == -1 || setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1 || bind(listenfd, this->ai_addr, this->ai_addrlen) == -1 || listen(listenfd, BACKLOG) == -1 || getsockname(listenfd, this->ai_addr, &this->ai_addrlen) == -1){
        fprintf(stderr, "Error: %s\n", strerror(errno));
        freeaddrinfo(this);
        exit(1);
    }

    /*
    The Linux Programming Interface
    Page 1213:
        "getaddrinfo() returns a list of socket address structures, each of which contains an IP address and port number"
    Page 1214:
        "The hints argument points to an addrinfo structure that specifies further criteria for selecting the socket address structures return via result"
        "addrinfo structures include a pointer to a socket address structure"
        struct addrinfo {
            int              ai_flags;
            int              ai_family;
            int              ai_socktype;
            int              ai_protocol;
            socklen_t        ai_addrlen;
            struct sockaddr *ai_addr;
            char            *ai_canonname;
            struct addrinfo *ai_next;
        };
    Page 1202:
        "An IPv4 socket address is stored as a sockaddr_in structure"
        struct sockaddr_in {
            short int            sin_family;
            unsigned short int   sin_port;
            struct in_addr       sin_addr;
            unsigned char        sin_zero[8];
        };
        "we saw that the generic sockaddr structure commences with a field identifying the socket domain. This corresponds to the sin_family field in the
        sockaddr_in structure, which is always set to AF_INET. The sin_port and sin_addr fields are the port number and the IP address, both in network byte order"
    Page 1204:
        "any type of socket address structure can be cast"
    */
    sprintf(ascii, "A%u", ntohs(((struct sockaddr_in *)this->ai_addr)->sin_port));

    freeaddrinfo(this);
    return listenfd;
}

void commandSet(char *commands[]){
    commands[0] = "Q"; commands[1] = "D"; commands[2] = "C";
    commands[3] = "L"; commands[4] = "G"; commands[5] = "P";
}

int analyizeCommand(const char *str){
    char *commands[6]; commandSet(commands);
    int i;
    for(i = 0; i < 6; i++) if(!strcmp(&str[0], commands[i])) break;
    return i;
}

void commandExe(int fd, int i, char *str){
    switch(i){
        case 0: // Q
            write(fd, "A", 2);
            exit(0);
        case 1: // D
            struct addrinfo *this = getAddr(NULL, "0");
            int listenfd = setupPort(this, str);
            write(fd, str, 7);

            int connectfd;
            struct sockaddr_storage clientAddr;
            socklen_t addrlen = sizeof(struct sockaddr_storage);
            if((connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &addrlen)) == -1){ // Accept connection
                fprintf(stderr, "Error: %s\n", strerror(errno));
                exit(1);
            }

            readParseAndLog(connectfd, str, 1);
            commandExe(connectfd, analyizeCommand(str), str);
            close(listenfd); close(connectfd);
            break;
        case 2: // C
            write(fd, "A", 2);
            break;
        case 3: // L
            write(fd, "A", 2);

            pid_t p;
            if(p = fork()){ // Parent
                waitpid(p, NULL, 0);
            } else { // Child
                if(dup2(fd, 1) == -1){ fprintf(stderr, "%s\n", strerror(errno)); exit(1); } // Writes to STDOUT
                if(close(fd) != 0){ fprintf(stderr, "%s\n", strerror(errno)); exit(1); }
                execlp("ls", "ls", "-l", NULL);
            }

            break;
        case 4: // G
            write(fd, "A", 2);
            break;
        case 5: // P
            write(fd, "A", 2);
            break;
        default:
            fprintf(stderr, "Error: Invalid Command\n");
    }
}

void main(int argc, char *argv[]){

    char host[NI_MAXHOST], buffer[BUFFER_SIZE];
    int connectfd, num;
    struct sockaddr_storage clientAddr;
    socklen_t addrlen = sizeof(struct sockaddr_storage); pid_t id;

    struct addrinfo *this = getAddr(NULL, "49999");
    int listenfd = setupPort(this, buffer);

    for(;;){
        if((connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &addrlen)) == -1){ // Accept connection
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(1);
        }
        
        if((id = fork()) == -1){ // Fork
            fprintf(stderr, "Error: %s\n", strerror(errno));
            close(connectfd); close(listenfd);
            exit(1);
        }
        else if(id){ // Parent
            close(connectfd);

            if((num = getnameinfo((struct sockaddr *) &clientAddr, addrlen, host, NI_MAXHOST, NULL, 0, NI_NUMERICSERV)) != 0){ // Convert socket addr to host name
                fprintf(stderr, "Error: %s\n", gai_strerror(num));
                exit(1);
            }

            fprintf(stdout, "Child %d: Connection Established: %s\n", id, host);

            if(waitpid(-1, NULL, WNOHANG) == -1){
                fprintf(stderr, "Error: %s\n", strerror(errno));
                exit(1);
            }
        }
        else{ // Child
            close(listenfd);

            for(;;){
                readParseAndLog(connectfd, buffer, 1);
                commandExe(connectfd, analyizeCommand(buffer), buffer);
            }
        }
    }
}
