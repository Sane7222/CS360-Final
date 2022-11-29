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

int connectAsClient(struct addrinfo *this){
    int connectfd;

    if((connectfd = socket(this->ai_family, this->ai_socktype, 0)) == -1 || connect(connectfd, this->ai_addr, this->ai_addrlen) == -1){ // Create a socket and connect it to server port
        fprintf(stderr, "Error: %s\n", strerror(errno));
        freeaddrinfo(this);
        exit(1);
    }

    freeaddrinfo(this);
    return connectfd;
}