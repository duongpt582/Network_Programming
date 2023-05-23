#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>

int main(int argc, char *argv[]){
       // Kiểm tra đầu vào
    if (argc != 4)
    {
        printf("Usage: %s <port_chat1> <port_chat2>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char ip_addr = *argv[1];
    int port_chat1 = atoi(argv[2]);
    int port_chat2 = atoi(argv[3]);

    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(&ip_addr);
    addr.sin_port = htons(port_chat1);


  
    int receiver =  socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in addrr;
    addrr.sin_family = AF_INET;
    addrr.sin_addr.s_addr = INADDR_ANY;
    addrr.sin_port = htons(port_chat2);
    bind(receiver, (struct sockaddr *)&addrr, sizeof(addrr));

    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(STDIN_FILENO, &fdread);
    FD_SET(receiver, &fdread);

    char buf[1024];
    while (1)
    {
        fdtest = fdread;
       
        int ret = select(receiver+1, &fdtest, NULL, NULL, NULL);
        // select se xoa nhung skien ko chay ra khoi fdread 

        if(ret<0){
            perror("select() failed");
            break;
        }
        
        if (FD_ISSET(STDIN_FILENO, &fdtest))
        // Kiểm tra sự kiện của fd xảy ra với tập set
        {
            fgets(buf, sizeof(buf), stdin);
            // send(receiver, buf, strlen(buf), 0);
            sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&addr, sizeof(addr));
        }
       
        if(FD_ISSET(receiver, &fdtest)){
            ret = recvfrom(receiver, buf, sizeof(buf), 0, NULL, NULL);
            buf[ret] = 0;
            printf("Received: %s\n", buf);
        }
    }
   
    return 1;
}