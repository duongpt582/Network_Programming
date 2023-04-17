#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    int port = atoi(argv[1]);
    char const *fileNameSend = argv[2];
    char const *fileNameReceive = argv[3];

    //create socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("Failed to create socket.\n");
        return 1;
    }

    //bind socket to port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("Failed to bind socket.\n");
        return 1;
    }

    //listen
    if (listen(listener, 5))
    {
        perror("Failed to listen.\n");
        return 1;
    }

    //accept
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client == -1)
    {
        perror("Failed to accept.\n");
        return 1;
    }

    printf("Accepted socket %d from IP: %s:%d\n\n", client, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    //open file have "hello" to send to client
    FILE *fileToSend = fopen(fileNameSend, "r");
    if (fileToSend == NULL)
    {
        perror("Failed to open file.\n");
        return 1;
    }

    //send text in file fileNameSend
    char buf[256];
    while (!feof(fileToSend))
    {
    
        fgets(buf, sizeof(buf), fileToSend);
        send(client, buf, strlen(buf), 0);
        printf("Sent to client: %s\n", buf);
    }

    
    //create file if not exist to receive from client
    FILE *fileToReceive = fopen(fileNameReceive, "a+");
    if (fileToReceive == NULL)
    {
        perror("Failed to open file.\n");
        return 1;
    }

    //receive text from client and write to file fileNameReceive
    while (1)
    {
        int n = recv(client, buf, sizeof(buf), 0);
        if (n == 0)
            break;
        buf[n] = '\0';
        printf("Received from client: %s\n", buf);
        fwrite(buf, 1, n, fileToReceive);
    }

    //close file
    fclose(fileToSend);
    fclose(fileToReceive);

    //close socket
    close(client);
    close(listener);

    return 0;
}
