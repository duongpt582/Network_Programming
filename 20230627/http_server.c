#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

void *thread_new_client(void *);
void serialize(buf, count, temp);
int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8686);

    int bind1 = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if (bind1)
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }
        printf("New client connected: %d \n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, thread_new_client, &client);
        pthread_detach(thread_id);
    }

    close(listener);

    return 0;
}

void *thread_new_client(void *param)
{
    char buf[4096];
    char api_method[32], uri[512];

    int client = *(int *)param;
    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);

    sscanf(buf, "%s%s", api_method, uri);

    if (strcmp(api_method, "GET") == 0)
    {
        if (strcmp(uri, "/") == 0)
        {
            DIR *dir;
            struct dirent *entry;
            char rel_dir[256] = ".";
            strcat(rel_dir, uri);
            dir = opendir(rel_dir);

            if (dir != NULL)
            {
                char response[4096] = "<html><body>";
                char *subresponse = malloc(512);
                while ((entry = readdir(dir)) != NULL)
                {
                    if (entry->d_type == DT_DIR)
                    {
                        sprintf(subresponse, "<a href=\"%s/\"><b>%s/</b></a><br>", entry->d_name, entry->d_name);
                    }
                    if (entry->d_type == DT_REG)
                    {
                        sprintf(subresponse, "<a href=\"%s\"><i>%s</i></a><br>", entry->d_name, entry->d_name);
                    }
                    strcat(response, subresponse);
                }
                strcat(response, "</html></body>");
                char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
                send(client, header, strlen(header), 0);
                send(client, response, strlen(response), 0);
            }
            else
            {
                char *response = "HTTP/1.1 404 File not found\r\nContent-Type: text/html\r\n\r\n<html><body><h1>opendir() failed</h1></body></html>";
                send(client, response, strlen(response), 0);
            }
        }
        else
        {
            DIR *dir;
            struct dirent *entry;
            char rel_dir[256] = ".";
            strcat(rel_dir, uri);
            dir = opendir(rel_dir);
            if (dir != NULL)
            {
                char response[4096] = "<html><body>";
                char *subresponse = malloc(512);
                while ((entry = readdir(dir)) != NULL)
                {
                    if (entry->d_type == DT_DIR)
                    {
                        sprintf(subresponse, "<a href=\"%s/\"><b>%s/</b></a><br>", entry->d_name, entry->d_name);
                    }
                    if (entry->d_type == DT_REG)
                    {
                        sprintf(subresponse, "<a href=\"%s\"><i>%s</i></a><br>", entry->d_name, entry->d_name);
                    }
                    strcat(response, subresponse);
                }
                strcat(response, "</html></body>");
                char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
                send(client, header, strlen(header), 0);
                send(client, response, strlen(response), 0);
            }
            else if (strstr(rel_dir, ".txt") != NULL || strstr(rel_dir, ".c") != NULL || strstr(rel_dir, ".cpp") != NULL)
            {
                char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
                send(client, header, strlen(header), 0);

                FILE *f = fopen(rel_dir, "rb");
                while (1)
                {
                    int len = fread(buf, 1, sizeof(buf), f);
                    if (len <= 0)
                        break;
                    send(client, buf, len, 0);
                }
                fclose(f);
            }
            else if (strstr(rel_dir, ".jpg") != NULL || strstr(rel_dir, ".png") != NULL)
            {
                FILE *f = fopen(rel_dir, "rb");

                char header[512];
                fseek(f, 0, SEEK_END);
                long file_size = ftell(f);
                fseek(f, 0, SEEK_SET);
                sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\n\r\n", file_size);
                send(client, header, strlen(header), 0);

                while (1)
                {
                    int len = fread(buf, 1, sizeof(buf), f);
                    if (len <= 0)
                        break;
                    send(client, buf, len, 0);
                }
                fclose(f);
            }
            else if (strstr(rel_dir, ".mp3") != NULL)
            {
                FILE *f = fopen(rel_dir, "rb");

                char header[512];
                fseek(f, 0, SEEK_END);
                unsigned long file_size = ftell(f);
                sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: audio/mpeg\r\nContent-Length: %ld\r\n\r\n", file_size);
                //Gui thong tin file mp3
                send(client, header, strlen(header), 0);

                //dua con tro ve dau file de tai file
                fseek(f, 0, SEEK_SET);
                
                
                while (1)
                {
                    int len = fread(buf, 1, sizeof(buf), f);
                    printf("length ok: %d\n", len);
                    if (len <= 0) {
                         printf("length break: %d\n", len);
                        break;

                    }

                    printf("done while\n");

                    send(client, buf, len, 0);
                }            

                fclose(f);
            }

        }

    }


    close(client);
}

void serialize(char *buffer, int count, char *data)
{
    int i=0, j=0;
    char temp1[20];

    sprintf(temp1, "%d", count);
    while(temp1[i] != '\0')
    {
        buffer[j++] = temp1[i++];
    }
    buffer[j++]=' ';

    for (i = 0; data[i] != '\0'; i++)
    {
        buffer[j++] = data[i];
    }
    buffer[j] = '\0';
    printf("BUFFER =%ld\n", sizeof(buffer));
}