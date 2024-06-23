#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <crypt.h>

#define PORT 8080

int sock = 0;
char logged_in_user[50] = {0};
char user_role[10] = {0};
char current_channel[50] = {0};
char current_room[50] = {0};

void handle_commands();
void register_user(char* username, char* password);
void login_user(char* username, char* password);
void list_channels();
void join_channel(char* channel, char* key);
void join_room(char* channel, char* room);
void send_chat(char* text);
void see_chat();
void create_channel(char* channel, char* key);
void edit_channel(char* old_channel, char* new_channel);
void delete_channel(char* channel);
void create_room(char* room);
void edit_room(char* old_room, char* new_room);
void delete_room(char* room);
void delete_all_rooms();
void exit_channel_or_room();

void register_user(char* username, char* password) {
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

    // Hash the password
    char salt[] = "$6$randomsalt$";  // SHA-512 salt format
    char *hashed_password = crypt(password, salt);

    sprintf(buffer, "REGISTER %s %s", username, hashed_password);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);

    if (strstr(buffer, "already registered")) {
        printf("%s sudah terdaftar\n", username);
    } else {
        printf("%s berhasil register\n", username);
    }

    close(sock);
}

void login_user(char* username, char* password) {
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

    // Hash the password with the same salt used during registration
    char salt[] = "$6$randomsalt$";  // Same salt used in registration
    char *hashed_password = crypt(password, salt);

    sprintf(buffer, "LOGIN %s %s", username, hashed_password);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);

    if (strstr(buffer, "User logged in successfully")) {
        strcpy(logged_in_user, username);
        sscanf(buffer, "%*s %*s %s", user_role); // Mendapatkan peran pengguna
        printf("%s berhasil login\n", username);
        handle_commands();  // Panggil fungsi setelah deklarasi
    } else {
        printf("%s\n", buffer);
        close(sock);
    }
}

void handle_commands() {
    char command[256];
    while (1) {
        if (strlen(current_room) > 0) {
            printf("[%s/%s/%s] ", logged_in_user, current_channel, current_room);
        } else if (strlen(current_channel) > 0) {
            printf("[%s/%s] ", logged_in_user, current_channel);
        } else {
            printf("[%s] ", logged_in_user);
        }
        
        fgets(command, sizeof(command), stdin);

        if (strncmp(command, "QUIT", 4) == 0) {
            printf("Logging out...\n");
            break;
        }

        if (strncmp(command, "JOIN ", 5) == 0) {
            char channel[50], key[50];
            sscanf(command + 5, "%s %s", channel, key);
            join_channel(channel, key);
            continue;
        }

        if (strncmp(command, "ROOM JOIN ", 10) == 0) {
            char room[50];
            sscanf(command + 10, "%s", room);
            join_room(current_channel, room);
            continue;
        }

        if (strncmp(command, "EXIT", 4) == 0) {
            exit_channel_or_room();
            continue;
        }

        if (strncmp(command, "LIST CHANNEL", 12) == 0) {
            list_channels();
            continue;
        }

        if (strncmp(command, "CREATE ROOM ", 12) == 0) {
            char room[50];
            sscanf(command + 12, "%s", room);
            create_room(room);
            continue;
        }

        send(sock, command, strlen(command), 0);
        char buffer[1024] = {0};
        read(sock, buffer, 1024);
        printf("%s\n", buffer);
    }
    close(sock);
}

void list_channels() {
    char buffer[1024] = {0};
    strcpy(buffer, "LIST CHANNEL");
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
}

void join_channel(char* channel, char* key) {
    char buffer[1024] = {0};
    sprintf(buffer, "JOIN %s %s", channel, key);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
    if (strstr(buffer, "Joined channel successfully")) {
        strcpy(current_channel, channel);
        memset(current_room, 0, sizeof(current_room));
    }
}

void join_room(char* channel, char* room) {
    char buffer[1024] = {0};
    sprintf(buffer, "ROOM JOIN %s %s", channel, room);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
    if (strstr(buffer, "Joined room successfully")) {
        strcpy(current_room, room);
    }
}

void send_chat(char* text) {
    char buffer[1024] = {0};
    sprintf(buffer, "CHAT %s", text);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
}

void see_chat() {
    char buffer[1024] = {0};
    strcpy(buffer, "SEE CHAT");
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
}

void create_channel(char* channel, char* key) {
    char buffer[1024] = {0};
    sprintf(buffer, "CREATE CHANNEL %s -k %s", channel, key);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
}

void edit_channel(char* old_channel, char* new_channel) {
    char buffer[1024] = {0};
    sprintf(buffer, "EDIT CHANNEL %s TO %s", old_channel, new_channel);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
}

void delete_channel(char* channel) {
    char buffer[1024] = {0};
    sprintf(buffer, "DEL CHANNEL %s", channel);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
}

void create_room(char* room) {
    char buffer[1024] = {0};
    if (strlen(current_channel) == 0) {
        printf("You need to join a channel first.\n");
        return;
    }
    sprintf(buffer, "CREATE ROOM %s %s", current_channel, room);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
}

void edit_room(char* old_room, char* new_room) {
    char buffer[1024] = {0};
    if (strlen(current_channel) == 0) {
        printf("You need to join a channel first.\n");
        return;
    }
    sprintf(buffer, "EDIT ROOM %s %s %s", current_channel, old_room, new_room);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
}

void delete_room(char* room) {
    char buffer[1024] = {0};
    if (strlen(current_channel) == 0) {
        printf("You need to join a channel first.\n");
        return;
    }
    sprintf(buffer, "DEL ROOM %s %s", current_channel, room);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
}

void delete_all_rooms() {
    char buffer[1024] = {0};
    if (strlen(current_channel) == 0) {
        printf("You need to join a channel first.\n");
        return;
    }
    sprintf(buffer, "DEL ROOM ALL %s", current_channel);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
}

void exit_channel_or_room() {
    char buffer[1024] = {0};
    if (strlen(current_room) > 0) {
        sprintf(buffer, "EXIT %s %s", current_channel, current_room);
    } else if (strlen(current_channel) > 0) {
        sprintf(buffer, "EXIT %s", current_channel);
    } else {
        strcpy(buffer, "EXIT");
    }
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);

    if (strstr(buffer, "Left room")) {
        memset(current_room, 0, sizeof(current_room));
    } else if (strstr(buffer, "Left channel")) {
        memset(current_channel, 0, sizeof(current_channel));
        memset(current_room, 0, sizeof(current_room)); // Clear room as well
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./discorit <command> [options]\n");
        return 1;
    }

    if (strcmp(argv[1], "REGISTER") == 0) {
        if (argc != 5 || strcmp(argv[3], "-p") != 0) {
            printf("Usage: ./discorit REGISTER <username> -p <password>\n");
            return 1;
        }
        register_user(argv[2], argv[4]);
    } else if (strcmp(argv[1], "LOGIN") == 0) {
        if (argc != 5 || strcmp(argv[3], "-p") != 0) {
            printf("Usage: ./discorit LOGIN <username> -p <password>\n");
            return 1;
        }
        login_user(argv[2], argv[4]);
    } else if (strcmp(argv[1], "LIST") == 0 && strcmp(argv[2], "CHANNEL") == 0) {
        list_channels();
    } else if (strcmp(argv[1], "JOIN") == 0) {
        if (argc != 4) {
            printf("Usage: ./discorit JOIN <channel> <key>\n");
            return 1;
        }
        join_channel(argv[2], argv[3]);
    } else if (strcmp(argv[1], "ROOM") == 0 && strcmp(argv[2], "JOIN") == 0) {
        join_room(current_channel, argv[3]);
    } else if (strcmp(argv[1], "CHAT") == 0) {
        send_chat(argv[2]);
    } else if (strcmp(argv[1], "SEE") == 0 && strcmp(argv[2], "CHAT") == 0) {
        see_chat();
    } else if (strcmp(argv[1], "CREATE") == 0 && strcmp(argv[2], "CHANNEL") == 0) {
        if (argc != 6 || strcmp(argv[4], "-k") != 0) {
            printf("Usage: ./discorit CREATE CHANNEL <channel> -k <key>\n");
            return 1;
        }
        create_channel(argv[3], argv[5]);
    } else if (strcmp(argv[1], "EDIT") == 0 && strcmp(argv[2], "CHANNEL") == 0) {
        if (argc != 5 || strcmp(argv[4], "TO") != 0) {
            printf("Usage: ./discorit EDIT CHANNEL <old_channel> TO <new_channel>\n");
            return 1;
        }
        edit_channel(argv[3], argv[5]);
    } else if (strcmp(argv[1], "DEL") == 0 && strcmp(argv[2], "CHANNEL") == 0) {
        delete_channel(argv[3]);
    } else if (strcmp(argv[1], "CREATE") == 0 && strcmp(argv[2], "ROOM") == 0) {
        create_room(argv[3]);
    } else if (strcmp(argv[1], "EDIT") == 0 && strcmp(argv[2], "ROOM") == 0) {
        if (argc != 5 || strcmp(argv[4], "TO") != 0) {
            printf("Usage: ./discorit EDIT ROOM <old_room> TO <new_room>\n");
            return 1;
        }
        edit_room(argv[3], argv[5]);
    } else if (strcmp(argv[1], "DEL") == 0 && strcmp(argv[2], "ROOM") == 0) {
        if (argc == 4 && strcmp(argv[3], "ALL") == 0) {
            delete_all_rooms();
        } else {
            delete_room(argv[3]);
        }
    } else {
        printf("Unknown command\n");
    }

    return 0;
}
