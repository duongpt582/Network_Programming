#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>

#define BUF_SIZE 1024
#define MAX_CLIENTS 10

typedef struct client
{
    int sockfd;
    struct sockaddr_in addr;
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
struct sockaddr_in server_addr;

void *client_proc(void *);
char listFile[100] = {"null"};
int countFile;

char *file_list;
int main(int argc, char const *argv[])
{

    DIR *dir;
    struct dirent *entry;

    dir = opendir("../20230606"); // Thay đổi "path/to/directory" thành đường dẫn tới thư mục của bạn
    if (dir == NULL)
    {
        perror("opendir");
        return 1;
    }
    countFile = 0;

    char *fileList;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            printf("day la File: %s\n", entry->d_name);
        }

        if (entry->d_type == DT_DIR)
        {
            printf("day la DIR: %s\n", entry->d_name);
        }
        // fileList = realloc(fileList, strlen(fileList) + strlen(entry->d_name) + 2 + 1);

        strcat(fileList, entry->d_name);
        strcat(fileList, "\n");
    }
    file_list = fileList;

    // Kiểm tra đầu vào
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Khởi tạo socket TCP
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    // Gán địa chỉ cho socket
    if (bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối
    if (listen(server, MAX_CLIENTS) < 0)
    {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for new client on %s:%d\n",
           inet_ntoa(server_addr.sin_addr),
           ntohs(server_addr.sin_port));

    while (1)
    {
        // Chấp nhận kết nối
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_len = sizeof(client_addr);
        int client = accept(server, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client < 0)
        {
            perror("accept() failed");
            exit(EXIT_FAILURE);
        }

        if (client_count == MAX_CLIENTS)
        {
            char *msg = "Server is full!\nPlease try again later.\n";
            if (send(client, msg, strlen(msg), 0) < 0)
            {
                perror("send() failed");
            }
            close(client);
            continue;
        }

        printf("New client connected from %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        pthread_mutex_lock(&clients_mutex);
        clients[client_count].sockfd = client;
        clients[client_count].addr = client_addr;
        client_count++;
        pthread_mutex_unlock(&clients_mutex);

        // Tạo thread để xử lý client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, client_proc, (void *)&clients[client_count - 1]) != 0)
        {
            perror("pthread_create() failed");
            exit(EXIT_FAILURE);
        }
        pthread_detach(thread_id);
    }

    // Đóng socket
    close(server);

    closedir(dir);
    return 0;
}

void *client_proc(void *param)
{
    client_t client = *(client_t *)param;

    // Xử lý client
    while (1)
    {
        // Gửi list file đến client
        if (send(client.sockfd, file_list, strlen(file_list), 0) < 0)
        {
            perror("send() failed");
            break;
        }

        // Nhận yêu cầu từ client
        char buf[BUF_SIZE];
        memset(buf, 0, BUF_SIZE);
        int len = recv(client.sockfd, buf, BUF_SIZE, 0);
        if (len < 0)
        {
            perror("recv() failed");
            break;
        }
        else if (len == 0)
        {
            printf("Client from %s:%d disconnected\n",
                   inet_ntoa(client.addr.sin_addr),
                   ntohs(client.addr.sin_port));
            for (int i = 0; i < client_count; i++)
            {
                if (client.sockfd == clients[i].sockfd)
                {
                    pthread_mutex_lock(&clients_mutex);
                    clients[i] = clients[client_count - 1];
                    client_count--;
                    if (client_count == 0)
                    {
                        printf("Waiting for clients on %s:%d...\n",
                               inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
                    }
                    pthread_mutex_unlock(&clients_mutex);
                    close(client.sockfd);
                    break;
                }
            }
            break;
        }
        else
        {
            // Xoá ký tự xuống dòng
            buf[strcspn(buf, "\n")] = 0;

            // Thoát nếu client gửi exit hoặc quit
            if (strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0)
            {
                char *msg = "Goodbye\n";
                if (send(client.sockfd, msg, strlen(msg), 0) < 0)
                {
                    perror("send() failed");
                    break;
                }

                printf("Client from %s:%d disconnected\n",
                       inet_ntoa(client.addr.sin_addr),
                       ntohs(client.addr.sin_port));
                for (int i = 0; i < client_count; i++)
                {
                    if (client.sockfd == clients[i].sockfd)
                    {
                        pthread_mutex_lock(&clients_mutex);
                        clients[i] = clients[client_count - 1];
                        client_count--;
                        if (client_count == 0)
                        {
                            printf("Waiting for clients on %s:%d...\n",
                                   inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
                        }
                        pthread_mutex_unlock(&clients_mutex);
                        close(client.sockfd);
                        break;
                    }
                }
                break;
            }

            // Xử lý yêu cầu
            char cmd[BUF_SIZE];
            char format[BUF_SIZE];
            char temp[BUF_SIZE];
            // int ret = sscanf(buf, "%s %s %s", cmd, format, temp);
            while (1)
            {
                int ret = recv(client.sockfd, buf, strlen(buf), 0);
                if (ret <= 0)
                    break;
                if (buf[ret - 1] == '\n')
                {
                    buf[ret - 1] = 0;
                }
                else
                {
                    buf[ret] = 0;
                }
                printf("Received from %d: %s\n", client.sockfd, buf);
                FILE *f = fopen("../20230606", "rb");
                if (f == NULL)
                {
                    char *msg = "Error no files to download\r\n";
                    send(client.sockfd, msg, strlen(msg), 0);
                    exit(0);
                }
                else
                {
                    fseek(f, 0, SEEK_END);
                    long file_size = ftell(f);
                    fseek(f, 0, SEEK_SET);
                    sprintf(buf, "OK %ld\r\n", file_size);
                    send(client.sockfd, buf, strlen(buf), 0);

                    while (!feof(f))
                    {
                        int ret = fread(buf, 1, sizeof(buf), f);
                        if (ret <= 0)
                            break;
                        send(client.sockfd, buf, ret, 0);
                    }
                    fclose(f);
                }
            }
        }
    }

    return NULL;
}
