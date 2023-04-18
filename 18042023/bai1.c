#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    // client
    int client = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6886);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("connect() failed");
        return 1;
    }

    char buffer[2048];
    char bufferTemp[2048];
    char computerName[1024];
    int totalDisk = 3;
    int diskSize;
    char diskName[1024];

    // enter computer name
    printf("Nhap ten may tinh: ");
    scanf("%s", computerName);
    getchar();
    strcat(buffer, computerName);
    strcat(buffer, "\n");

    for (int i = 0; i < totalDisk; i++)
    {
        printf("Nhap o dia thu %d: ", i+1);
        scanf("%s", diskName);
        getchar();

        printf("Nhap dung luong o dia %d: ", i+1);
        scanf("%d", &diskSize);
        getchar();

        sprintf(bufferTemp, "%s - %dGB\n", diskName, diskSize);
        strcat(buffer, bufferTemp);
    }

    printf("%s\n", buffer);

    send(client, buffer, strlen(buffer), 0);
    
    close(client);
    
    return 0;
}
