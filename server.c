#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <crypt.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PORT 8080

void *client_handler(void *socket_desc);
void handle_register(int client_socket, char *username, char *password);
void handle_login(int client_socket, char *username, char *password);
void handle_list_channels(int client_socket);
void handle_join_channel(int client_socket, char *channel);
void handle_join_room(int client_socket, char *room);
void handle_send_chat(int client_socket, char *text);
void handle_see_chat(int client_socket);
void handle_create_channel(int client_socket, char *channel, char *key);
void handle_edit_channel(int client_socket, char *old_channel, char *new_channel);
void handle_delete_channel(int client_socket, char *channel);
void handle_create_room(int client_socket, char *room);
void handle_edit_room(int client_socket, char *old_room, char *new_room);
void handle_delete_room(int client_socket, char *room);
void handle_delete_all_rooms(int client_socket);

void server_daemon() {
    int server_socket, client_socket, c, *new_sock;
    struct sockaddr_in server, client;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        printf("Could not create socket\n");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("bind failed. Error");
        return;
    }

    listen(server_socket, 3);

    printf("Waiting for incoming connections...\n");
    c = sizeof(struct sockaddr_in);

    while ((client_socket = accept(server_socket, (struct sockaddr*)&client, (socklen_t*)&c))) {
        printf("Connection accepted\n");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_socket;

        if (pthread_create(&sniffer_thread, NULL, client_handler, (void*)new_sock) < 0) {
            perror("could not create thread");
            return;
        }

        printf("Handler assigned\n");
    }

    if (client_socket < 0) {
        perror("accept failed");
        return;
    }
}

void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    int read_size;
    char client_message[2000];

    while ((read_size = recv(sock, client_message, 2000, 0)) > 0) {
        client_message[read_size] = '\0';
        printf("Received: %s\n", client_message);

        char *command = strtok(client_message, " ");
        if (strcmp(command, "REGISTER") == 0) {
            char *username = strtok(NULL, " ");
            char *password = strtok(NULL, " ");
            handle_register(sock, username, password);
        } else if (strcmp(command, "LOGIN") == 0) {
            char *username = strtok(NULL, " ");
            char *password = strtok(NULL, " ");
            handle_login(sock, username, password);
        } else if (strcmp(command, "LIST") == 0) {
            char *subcommand = strtok(NULL, " ");
            if (strcmp(subcommand, "CHANNEL") == 0) {
                handle_list_channels(sock);
            }
        } else if (strcmp(command, "JOIN") == 0) {
            char *channel = strtok(NULL, " ");
            handle_join_channel(sock, channel);
        } else if (strcmp(command, "ROOM") == 0) {
            char *subcommand = strtok(NULL, " ");
            if (strcmp(subcommand, "JOIN") == 0) {
                char *room = strtok(NULL, " ");
                handle_join_room(sock, room);
            }
        } else if (strcmp(command, "CHAT") == 0) {
            char *text = strtok(NULL, "");
            handle_send_chat(sock, text);
        } else if (strcmp(command, "SEE") == 0) {
            char *subcommand = strtok(NULL, " ");
            if (strcmp(subcommand, "CHAT") == 0) {
                handle_see_chat(sock);
            }
        } else if (strcmp(command, "CREATE") == 0) {
            char *type = strtok(NULL, " ");
            if (strcmp(type, "CHANNEL") == 0) {
                char *channel = strtok(NULL, " ");
                char *key_flag = strtok(NULL, " ");
                char *key = strtok(NULL, " ");
                if (strcmp(key_flag, "-k") == 0) {
                    handle_create_channel(sock, channel, key);
                }
            } else if (strcmp(type, "ROOM") == 0) {
                char *room = strtok(NULL, " ");
                handle_create_room(sock, room);
            }
        } else if (strcmp(command, "EDIT") == 0) {
            char *type = strtok(NULL, " ");
            if (strcmp(type, "CHANNEL") == 0) {
                char *old_channel = strtok(NULL, " ");
                strtok(NULL, " ");
                char *new_channel = strtok(NULL, " ");
                handle_edit_channel(sock, old_channel, new_channel);
            } else if (strcmp(type, "ROOM") == 0) {
                char *old_room = strtok(NULL, " ");
                strtok(NULL, " ");
                char *new_room = strtok(NULL, " ");
                handle_edit_room(sock, old_room, new_room);
            }
        } else if (strcmp(command, "DEL") == 0) {
            char *type = strtok(NULL, " ");
            if (strcmp(type, "CHANNEL") == 0) {
                char *channel = strtok(NULL, " ");
                handle_delete_channel(sock, channel);
            } else if (strcmp(type, "ROOM") == 0) {
                char *room = strtok(NULL, " ");
                if (strcmp(room, "ALL") == 0) {
                    handle_delete_all_rooms(sock);
                } else {
                    handle_delete_room(sock, room);
                }
            }
        } else {
            char response[] = "Unknown command\n";
            write(sock, response, strlen(response));
        }

        memset(client_message, 0, sizeof(client_message));
    }

    if (read_size == 0) {
        printf("Client disconnected\n");
    } else if (read_size == -1) {
        perror("recv failed");
    }

    free(socket_desc);
    return 0;
}

void handle_register(int client_socket, char *username, char *password) {
    FILE *file = fopen("users.csv", "r+");
    if (!file) {
        file = fopen("users.csv", "w");
        if (file) {
            fprintf(file, "id_user,name,password,global_role\n");
        } else {
            char response[] = "Error: Could not open or create users.csv\n";
            write(client_socket, response, strlen(response));
            return;
        }
    }

    char line[256];
    int user_count = 0;  // Initialize user_count to 0
    int max_id = 0;      // Track the highest id_user

    while (fgets(line, sizeof(line), file)) {
        if (user_count == 0) {
            user_count++;
            continue;  // Skip header
        }
        
        // Parse the line
        char *id_str = strtok(line, ",");
        char *stored_username = strtok(NULL, ",");

        // Check if the username already exists
        if (strcmp(stored_username, username) == 0) {
            char response[] = "Username already registered\n";
            write(client_socket, response, strlen(response));
            fclose(file);
            return;
        }
        
        // Update the max_id
        int id = atoi(id_str);
        if (id > max_id) {
            max_id = id;
        }

        user_count++;
    }

    // Hash the password
    char salt[] = "$6$randomsalt$";  // SHA-512 salt format
    char *hashed_password = crypt(password, salt);
    const char *role = (user_count == 0) ? "ROOT" : "USER";  // Pengguna pertama setelah header adalah ROOT

    fprintf(file, "%d,%s,%s,%s\n", max_id + 1, username, hashed_password, role);
    fclose(file);

    char response[] = "User registered successfully\n";
    write(client_socket, response, strlen(response));
}

void handle_login(int client_socket, char *username, char *password) {
    FILE *file = fopen("users.csv", "r");
    if (!file) {
        char response[] = "Error: Could not open users.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char line[256];
    fgets(line, sizeof(line), file);  // Skip header
    while (fgets(line, sizeof(line), file)) {
        strtok(line, ",");  // Skip the id
        char *stored_username = strtok(NULL, ",");
        char *stored_password = strtok(NULL, ",");
        char *global_role = strtok(NULL, ",");

        if (strcmp(stored_username, username) == 0) {
            char *hashed_password = crypt(password, stored_password);

            if (strcmp(hashed_password, stored_password) == 0) {
                char response[1024];
                snprintf(response, sizeof(response), "User logged in successfully %s\n", global_role);
                write(client_socket, response, strlen(response));
                fclose(file);
                return;
            } else {
                char response[] = "Incorrect password\n";
                write(client_socket, response, strlen(response));
                fclose(file);
                return;
            }
        }
    }

    char response[] = "Username not found\n";
    write(client_socket, response, strlen(response));
    fclose(file);
}

void handle_create_channel(int client_socket, char *channel, char *key) {
    FILE *file = fopen("channels.csv", "r+");
    if (!file) {
        file = fopen("channels.csv", "w");
        if (file) {
            fprintf(file, "id_channel,channel,key\n");
        } else {
            char response[] = "Error: Could not open or create channels.csv\n";
            write(client_socket, response, strlen(response));
            return;
        }
    }

    int channel_count = 1;  // Start from 1 because 1st line is header
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (channel_count == 1) {
            channel_count++;
            continue;  // Skip header
        }
        channel_count++;
    }

    char salt[] = "$6$randomsalt$";  // SHA-512 salt format
    char *hashed_key = crypt(key, salt);

    fprintf(file, "%d,%s,%s\n", channel_count - 1, channel, hashed_key);  // Subtract 1 to exclude header line
    fclose(file);

    char admin_path[512];
    snprintf(admin_path, sizeof(admin_path), "%s/admin", channel);

    if (mkdir(channel, 0700) == -1 || mkdir(admin_path, 0700) == -1) {
        char response[] = "Error: Could not create channel directories\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char user_log_path[1024];
    snprintf(user_log_path, sizeof(user_log_path), "%s/user.log", admin_path);
    FILE *user_log_file = fopen(user_log_path, "w");
    if (!user_log_file) {
        char response[] = "Error: Could not create user.log\n";
        write(client_socket, response, strlen(response));
        return;
    }
    fclose(user_log_file);

    char auth_path[1024];
    snprintf(auth_path, sizeof(auth_path), "%s/auth.csv", admin_path);
    FILE *auth_file = fopen(auth_path, "w");
    if (!auth_file) {
        char response[] = "Error: Could not create auth.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }
    fprintf(auth_file, "id_user,name,role\n");
    fclose(auth_file);

    char response[256];
    snprintf(response, sizeof(response), "Channel %s created successfully\n", channel);
    write(client_socket, response, strlen(response));
}

void handle_edit_channel(int client_socket, char *old_channel, char *new_channel) {
    FILE *file = fopen("channels.csv", "r");
    if (!file) {
        char response[] = "Error: Could not open channels.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char temp_path[] = "channels_temp.csv";
    FILE *temp_file = fopen(temp_path, "w");
    if (!temp_file) {
        char response[] = "Error: Could not create temporary file\n";
        write(client_socket, response, strlen(response));
        fclose(file);
        return;
    }

    int found = 0;
    int channel_exists = 0;
    char line[256];

    // Write the header to the temp file
    fgets(line, sizeof(line), file);
    fputs(line, temp_file);

    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;

        char *id = strtok(line, ",");
        char *channel_name = strtok(NULL, ",");
        char *key = strtok(NULL, ",");

        if (channel_name && strcmp(channel_name, new_channel) == 0) {
            channel_exists = 1;
            break;
        }
        if (channel_name && strcmp(channel_name, old_channel) == 0) {
            found = 1;
            fprintf(temp_file, "%s,%s,%s\n", id, new_channel, key ? key : "");
        } else {
            fprintf(temp_file, "%s,%s,%s\n", id, channel_name, key ? key : "");
        }
    }
    fclose(file);
    fclose(temp_file);

    if (channel_exists) {
        remove(temp_path);
        char response[] = "Nama channel sudah digunakan\n";
        write(client_socket, response, strlen(response));
        return;
    }

    if (found) {
        if (remove("channels.csv") != 0) {
            char response[] = "Error: Could not remove original file\n";
            write(client_socket, response, strlen(response));
            return;
        }
        if (rename(temp_path, "channels.csv") != 0) {
            char response[] = "Error: Could not rename temporary file\n";
            write(client_socket, response, strlen(response));
            return;
        }

        // Rename the directory
        char old_channel_path[256];
        char new_channel_path[256];
        snprintf(old_channel_path, sizeof(old_channel_path), "%s", old_channel);
        snprintf(new_channel_path, sizeof(new_channel_path), "%s", new_channel);

        if (rename(old_channel_path, new_channel_path) != 0) {
            char response[] = "Error: Could not rename channel directory\n";
            write(client_socket, response, strlen(response));
            return;
        }

        char response[256];
        snprintf(response, sizeof(response), "Channel %s berhasil diubah menjadi %s\n", old_channel, new_channel);
        write(client_socket, response, strlen(response));
    } else {
        remove(temp_path);
        char response[256];
        snprintf(response, sizeof(response), "Channel %s tidak ditemukan\n", old_channel);
        write(client_socket, response, strlen(response));
    }
}

void handle_delete_channel(int client_socket, char *channel) {
    FILE *file = fopen("channels.csv", "r");
    if (!file) {
        char response[] = "Error: Could not open channels.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char temp_path[] = "channels_temp.csv";
    FILE *temp_file = fopen(temp_path, "w");
    if (!temp_file) {
        char response[] = "Error: Could not create temporary file\n";
        write(client_socket, response, strlen(response));
        fclose(file);
        return;
    }

    int found = 0;
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        char *id = strtok(line, ",");
        char *channel_name = strtok(NULL, ",");
        char *key = strtok(NULL, "\n");

        if (channel_name && strcmp(channel_name, channel) == 0) {
            found = 1;
            continue;  // Skip writing this channel to temp file
        }
        fprintf(temp_file, "%s,%s,%s\n", id, channel_name, key ? key : "");
    }
    fclose(file);
    fclose(temp_file);

    if (found) {
        if (remove("channels.csv") != 0) {
            char response[] = "Error: Could not remove original file\n";
            write(client_socket, response, strlen(response));
            return;
        }
        if (rename(temp_path, "channels.csv") != 0) {
            char response[] = "Error: Could not rename temporary file\n";
            write(client_socket, response, strlen(response));
            return;
        }

        // Remove the directory
        char channel_path[256];
        snprintf(channel_path, sizeof(channel_path), "%s", channel);
        char command[512];
        snprintf(command, sizeof(command), "rm -rf %s", channel_path);
        system(command);

        char response[256];
        snprintf(response, sizeof(response), "Channel %s successfully deleted\n", channel);
        write(client_socket, response, strlen(response));
    } else {
        remove(temp_path);
        char response[256];
        snprintf(response, sizeof(response), "Channel %s not found\n", channel);
        write(client_socket, response, strlen(response));
    }
}

void handle_create_room(int client_socket, char *room) {
    FILE *file = fopen("rooms.csv", "r+");
    if (!file) {
        file = fopen("rooms.csv", "w");
        if (file) {
            fprintf(file, "id_room,room\n");
        } else {
            char response[] = "Error: Could not open or create rooms.csv\n";
            write(client_socket, response, strlen(response));
            return;
        }
    }

    int room_count = 1;  // Start from 1 because 1st line is header
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (room_count == 1) {
            room_count++;
            continue;  // Skip header
        }
        room_count++;
    }

    fprintf(file, "%d,%s\n", room_count - 1, room);  // Subtract 1 to exclude header line
    fclose(file);

    char response[256];
    snprintf(response, sizeof(response), "Room %s created successfully\n", room);
    write(client_socket, response, strlen(response));
}

void handle_edit_room(int client_socket, char *old_room, char *new_room) {
    FILE *file = fopen("rooms.csv", "r");
    if (!file) {
        char response[] = "Error: Could not open rooms.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char temp_path[] = "rooms_temp.csv";
    FILE *temp_file = fopen(temp_path, "w");
    if (!temp_file) {
        char response[] = "Error: Could not create temporary file\n";
        write(client_socket, response, strlen(response));
        fclose(file);
        return;
    }

    int found = 0;
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        char *id = strtok(line, ",");
        char *room_name = strtok(NULL, "\n");

        if (room_name && strcmp(room_name, old_room) == 0) {
            found = 1;
            fprintf(temp_file, "%s,%s\n", id, new_room);
        } else {
            fprintf(temp_file, "%s,%s\n", id, room_name);
        }
    }
    fclose(file);
    fclose(temp_file);

    if (found) {
    if (remove("rooms.csv") != 0) {
        char response[] = "Error: Could not remove original file\n";
        write(client_socket, response, strlen(response));
        return;
    }
    if (rename(temp_path, "rooms.csv") != 0) {
        char response[] = "Error: Could not rename temporary file\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char response[256];
    snprintf(response, sizeof(response), "Room %s berhasil diubah menjadi %s\n", old_room, new_room);
    write(client_socket, response, strlen(response));
} else {
    remove(temp_path); // This is the correct argument
    char response[256];
    snprintf(response, sizeof(response), "Room %s tidak ditemukan\n", old_room);
    write(client_socket, response, strlen(response));
}

}

void handle_delete_room(int client_socket, char *room) {
    FILE *file = fopen("rooms.csv", "r");
    if (!file) {
        char response[] = "Error: Could not open rooms.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char temp_path[] = "rooms_temp.csv";
    FILE *temp_file = fopen(temp_path, "w");
    if (!temp_file) {
        char response[] = "Error: Could not create temporary file\n";
        write(client_socket, response, strlen(response));
        fclose(file);
        return;
    }

    int found = 0;
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        char *id = strtok(line, ",");
        char *room_name = strtok(NULL, "\n");

        if (room_name && strcmp(room_name, room) == 0) {
            found = 1;
            continue;  // Skip writing this room to temp file
        }
        fprintf(temp_file, "%s,%s\n", id, room_name);
    }
    fclose(file);
    fclose(temp_file);

    if (found) {
        if (remove("rooms.csv") != 0) {
            char response[] = "Error: Could not remove original file\n";
            write(client_socket, response, strlen(response));
            return;
        }
        if (rename(temp_path, "rooms.csv") != 0) {
            char response[] = "Error: Could not rename temporary file\n";
            write(client_socket, response, strlen(response));
            return;
        }
        char response[256];
        snprintf(response, sizeof(response), "Room %s successfully deleted\n", room);
        write(client_socket, response, strlen(response));
    } else {
        remove(temp_path);
        char response[256];
        snprintf(response, sizeof(response), "Room %s not found\n", room);
        write(client_socket, response, strlen(response));
    }
}

void handle_delete_all_rooms(int client_socket) {
    FILE *file = fopen("rooms.csv", "w");
    if (!file) {
        char response[] = "Error: Could not open rooms.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    fprintf(file, "id_room,room\n");  // Write header
    fclose(file);

    char response[] = "All rooms successfully deleted\n";
    write(client_socket, response, strlen(response));
}

void handle_list_channels(int client_socket) {
    FILE *file = fopen("channels.csv", "r");
    if (!file) {
        char response[] = "Error: Could not open channels.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char line[256];
    char response[1024] = "Channels:\n";
    fgets(line, sizeof(line), file);  // Skip header
    while (fgets(line, sizeof(line), file)) {
        char *id = strtok(line, ",");
        char *channel = strtok(NULL, ",");
        strcat(response, channel);
        strcat(response, "\n");
    }

    write(client_socket, response, strlen(response));
    fclose(file);
}

void handle_join_channel(int client_socket, char *channel) {
    FILE *file = fopen("channels.csv", "r");
    if (!file) {
        char response[] = "Error: Could not open channels.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char line[256];
    fgets(line, sizeof(line), file);  // Skip header
    while (fgets(line, sizeof(line), file)) {
        char *id = strtok(line, ",");
        char *channel_name = strtok(NULL, ",");

        if (strcmp(channel_name, channel) == 0) {
            char response[] = "Joined channel successfully\n";
            write(client_socket, response, strlen(response));
            fclose(file);
            return;
        }
    }

    char response[] = "Channel not found\n";
    write(client_socket, response, strlen(response));
    fclose(file);
}

void handle_join_room(int client_socket, char *room) {
    char response[] = "Joined room successfully\n";
    write(client_socket, response, strlen(response));
}

void handle_send_chat(int client_socket, char *text) {
    FILE *file = fopen("chat.csv", "a+");
    if (!file) {
        char response[] = "Error: Could not open chat.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    time_t now = time(NULL);
    fprintf(file, "%ld,%d,%s,%s\n", now, 1, "sender", text); // Sender diambil dari sesi pengguna
    fclose(file);

    char response[] = "Chat sent\n";
    write(client_socket, response, strlen(response));
}

void handle_see_chat(int client_socket) {
    FILE *file = fopen("chat.csv", "r");
    if (!file) {
        char response[] = "Error: Could not open chat.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char line[256];
    char response[1024] = "";
    while (fgets(line, sizeof(line), file)) {
        strcat(response, line);
    }

    write(client_socket, response, strlen(response));
    fclose(file);
}

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        printf("Error in forking process\n");
        exit(1);
    }

    if (pid > 0) {
        exit(0);
    }

    if (setsid() < 0) {
        exit(1);
    }

    signal(SIGCHLD, SIG_IGN);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    server_daemon();

    return 0;
}
