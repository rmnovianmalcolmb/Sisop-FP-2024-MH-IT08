#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

void monitor_chat(char* username, char* channel, char* room) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address / Address not supported\n");
        return;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return;
    }

    sprintf(buffer, "MONITOR %s %s %s", username, channel, room);
    send(sock, buffer, strlen(buffer), 0);

    while (1) {
        int valread = read(sock, buffer, 1024);
        if (valread > 0) {
            printf("%s\n", buffer);
            memset(buffer, 0, sizeof(buffer));
        }
    }

    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        printf("Usage: ./monitor <username> -channel <channel_name> -room <room_name>\n");
        return 1;
    }

    if (strcmp(argv[2], "-channel") != 0 || strcmp(argv[4], "-room") != 0) {
        printf("Usage: ./monitor <username> -channel <channel_name> -room <room_name>\n");
        return 1;
    }

    monitor_chat(argv[1], argv[3], argv[5]);

    return 0;
}
