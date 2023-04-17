#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main(int argv, char *argc[]) {


    // argv: so tham so dau vao
    // argc[i]: gia tri cua tung vi tri tham so
    printf("So tham so dau vao: %d.\n", argv);
    printf("Gia tri cua tham so thu 1: %s\n", argc[1]);
    printf("Gia tri cua tham so thu 2: %s\n", argc[2]);

    int port = atoi(argc[2]);
    printf("port: %d\n", port);
    char *ip_addr = argc[1];

    // Tao 1 socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET; //IPv4
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(port);

    int ret = connect(client, (struct sockaddr*) &addr, sizeof(addr));
    if (ret == -1)
    {
        perror("Failed to create client.\n connect() failed\n");
        return 1;
    } 

    char *msg = "Hello Server";
    send(client, msg, strlen(msg), 0);

    //receive data from server
    char buf[256];
    int recerve = recv(client, buf, sizeof(buf), 0);
    if (recerve == -1)
    {
        perror("Failed to receive data.\n");
        
    }
    else if (recerve == 0)
    {
        printf("Server closed connection.\n");
    }
    else
    {
        printf("Received from server: %s\n", buf);
    }

    //send data to server
    // char buf[256];
    while (1)
    {
        printf("Nhap du lieu: ");
        fgets(buf, sizeof(buf), stdin);
        send(client, buf, strlen(buf), 0);
        if (strncmp(buf, "exit", 4) == 0)
            break;

    }

    close(client);
    return 0;


}