#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    // server
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(6886);

    // bind
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    // listen
    if(listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

     struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    int server = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (server == -1)
    {
        perror("accept() failed");
        return 1;
    }
    printf("New client connected: %d\n", server);


    
    char buffer[2048];
    char bufferTemp[2048];
    char computerName[1024];
    int totalDisk = 3;
    int diskSize;
    char diskName[1024];

    while (1)
    {
        int ret = recv(server, buffer, sizeof(buffer), 0);
        if (ret <= 0)
        {
            perror("recv() failed");
            break;
        }
        printf("Received:\n %s\n", buffer);
    }

    // close(server);
    // close(listener);
    
    



    return 0;
}
