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
#include <dirent.h>

#define PORT 8080
#define BASE_DIR "DiscorIT"

void *client_handler(void *socket_desc);
void handle_register(int client_socket, char *username, char *password);
void handle_login(int client_socket, char *username, char *password);
void handle_list_channels(int client_socket);
void handle_join_channel(int client_socket, char *channel, char *hashed_key);
void handle_join_room(int client_socket, char *channel, char *room);
void handle_send_chat(int client_socket, char *text);
void handle_see_chat(int client_socket);
void handle_create_channel(int client_socket, char *channel, char *key, char *creator_username);
void handle_edit_channel(int client_socket, char *old_channel, char *new_channel);
void handle_delete_channel(int client_socket, char *channel);
void handle_create_room(int client_socket, char *channel, char *room);
void handle_edit_room(int client_socket, char *channel, char *old_room, char *new_room);
void handle_delete_room(int client_socket, char *channel, char *room);
void handle_delete_all_rooms(int client_socket);
void handle_exit(int client_socket, char *channel, char *room);
void handle_remove_user(int client_socket, char *channel, char *username);
void handle_ban_user(int client_socket, char *channel, char *username);
void handle_unban_user(int client_socket, char *channel, char *username);

void create_base_directory() {
    struct stat st = {0};
    if (stat(BASE_DIR, &st) == -1) {
        mkdir(BASE_DIR, 0700);
    }
}

void server_daemon() {
    create_base_directory();

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

    char current_user[256] = "";
    char current_channel[256] = "";
    char current_room[256] = "";

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
            strncpy(current_user, username, sizeof(current_user));
        } else if (strcmp(command, "LIST") == 0) {
            char *subcommand = strtok(NULL, " ");
            if (strcmp(subcommand, "CHANNEL") == 0) {
                handle_list_channels(sock);
            }
        } else if (strcmp(command, "JOIN") == 0) {
            char *first_arg = strtok(NULL, " ");
            char *second_arg = strtok(NULL, " ");
            if (strlen(current_channel) > 0){
                // Handle as room join if second argument is NULL
                handle_join_room(sock, current_channel, second_arg);
                strncpy(current_room, first_arg, sizeof(current_room));
            } else {
                // Handle as channel join if second argument is not NULL
                handle_join_channel(sock, first_arg, second_arg);
                strncpy(current_channel, first_arg, sizeof(current_channel));
                memset(current_room, 0, sizeof(current_room)); // Clear current room
                second_arg = NULL;
            }
        } else if (strcmp(command, "EXIT") == 0) {
            char *channel = strtok(NULL, " ");
            char *room = strtok(NULL, " ");
            if (channel == NULL && room == NULL) {
                handle_exit(sock, NULL, NULL);
            } else {
                handle_exit(sock, channel, room);
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
                    handle_create_channel(sock, channel, key, current_user);
                }
            } else if (strcmp(type, "ROOM") == 0) {
                char *channel = strtok(NULL, " ");
                char *room = strtok(NULL, " ");
                handle_create_room(sock, channel, room);
            }
        } else if (strcmp(command, "EDIT") == 0) {
            char *type = strtok(NULL, " ");
            if (strcmp(type, "CHANNEL") == 0) {
                char *old_channel = strtok(NULL, " ");
                strtok(NULL, " ");
                char *new_channel = strtok(NULL, " ");
                handle_edit_channel(sock, old_channel, new_channel);
            } else if (strcmp(type, "ROOM") == 0) {
                char *channel = strtok(NULL, " ");
                char *old_room = strtok(NULL, " ");
                strtok(NULL, " ");
                char *new_room = strtok(NULL, " ");
                handle_edit_room(sock, channel, old_room, new_room);
            }
        } else if (strcmp(command, "DEL") == 0) {
            char *type = strtok(NULL, " ");
            if (strcmp(type, "CHANNEL") == 0) {
                char *channel = strtok(NULL, " ");
                handle_delete_channel(sock, channel);
            } else if (strcmp(type, "ROOM") == 0) {
                char *channel = strtok(NULL, " ");
                char *room = strtok(NULL, " ");
                if (strcmp(room, "ALL") == 0) {
                    handle_delete_all_rooms(sock);
                } else {
                    handle_delete_room(sock, channel, room);
                }
            }
        } else if (strcmp(command, "REMOVE") == 0) {
            char *channel = strtok(NULL, " ");
            char *username = strtok(NULL, " ");
            handle_remove_user(sock, channel, username);
        } else if (strcmp(command, "BAN") == 0) {
            char *channel = strtok(NULL, " ");
            char *username = strtok(NULL, " ");
            handle_ban_user(sock, channel, username);
        } else if (strcmp(command, "UNBAN") == 0) {
            char *channel = strtok(NULL, " ");
            char *username = strtok(NULL, " ");
            handle_unban_user(sock, channel, username);
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
    char path[256];
    snprintf(path, sizeof(path), "%s/users.csv", BASE_DIR);
    FILE *file = fopen(path, "r+");
    if (!file) {
        file = fopen(path, "w");
        if (!file) {
            char response[] = "Error: Could not open or create users.csv\n";
            write(client_socket, response, strlen(response));
            return;
        }
    }

    char line[256];
    int user_count = 0;  // Initialize user_count to 0
    int max_id = 0;      // Track the highest id_user

    while (fgets(line, sizeof(line), file)) {
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
    char path[256];
    snprintf(path, sizeof(path), "%s/users.csv", BASE_DIR);
    FILE *file = fopen(path, "r");
    if (!file) {
        char response[] = "Error: Could not open users.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char line[256];
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

void handle_create_channel(int client_socket, char *channel, char *key, char *creator_username) {
    char path[256];
    snprintf(path, sizeof(path), "%s/channels.csv", BASE_DIR);
    FILE *file = fopen(path, "r+");
    if (!file) {
        file = fopen(path, "w");
        if (!file) {
            char response[] = "Error: Could not open or create channels.csv\n";
            write(client_socket, response, strlen(response));
            return;
        }
    }

    int channel_count = 1;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        channel_count++;
    }

    char salt[] = "$6$randomsalt$";  // SHA-512 salt format
    char *hashed_key = crypt(key, salt);

    fprintf(file, "%d,%s,%s\n", channel_count, channel, hashed_key);
    fclose(file);

    char admin_path[512];
    snprintf(admin_path, sizeof(admin_path), "%s/%s/admin", BASE_DIR, channel);

    char channel_path[512];
    snprintf(channel_path, sizeof(channel_path), "%s/%s", BASE_DIR, channel);

    if (mkdir(channel_path, 0700) == -1 || mkdir(admin_path, 0700) == -1) {
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
    fprintf(auth_file, "1,%s,ADMIN\n", creator_username); // Make the creator an admin
    fclose(auth_file);

    char response[256];
    snprintf(response, sizeof(response), "Channel %s dibuat\n", channel);
    write(client_socket, response, strlen(response));
}

void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        if (len > 1 && str[len - 2] == '\r') {
            str[len - 2] = '\0';
        }
    }
}

void handle_edit_channel(int client_socket, char *old_channel, char *new_channel) {
    char path[256];
    snprintf(path, sizeof(path), "%s/channels.csv", BASE_DIR);
    FILE *file = fopen(path, "r");
    FILE *temp_file = fopen("channels_temp.csv", "w");
    if (!file || !temp_file) {
        char response[] = "Error: Could not open channels.csv or create temporary file\n";
        write(client_socket, response, strlen(response));
        if (file) fclose(file);
        if (temp_file) fclose(temp_file);
        return;
    }

    // Trim any newline characters from new_channel
    trim_newline(new_channel);

    char line[256];
    int found = 0;
    int channel_exists = 0;

    while (fgets(line, sizeof(line), file)) {
        int id;
        char stored_channel[256], key[256];
        sscanf(line, "%d,%[^,],%s", &id, stored_channel, key);

        // Trim any newline characters from stored_channel and key
        stored_channel[strcspn(stored_channel, "\n")] = 0;
        key[strcspn(key, "\n")] = 0;

        // Check if the new channel name already exists
        if (strcmp(stored_channel, new_channel) == 0) {
            channel_exists = 1;
            break;
        }
        if (strcmp(stored_channel, old_channel) == 0) {
            fprintf(temp_file, "%d,%s,%s\n", id, new_channel, key);
            found = 1;
        } else {
            fprintf(temp_file, "%d,%s,%s\n", id, stored_channel, key);
        }
    }
    fclose(file);
    fclose(temp_file);

    if (channel_exists) {
        remove("channels_temp.csv");
        char response[] = "Nama channel sudah digunakan\n";
        write(client_socket, response, strlen(response));
        return;
    }

    if (found) {
        remove(path);
        rename("channels_temp.csv", path);

        // Rename the directory
        char old_channel_path[512];
        char new_channel_path[512];
        snprintf(old_channel_path, sizeof(old_channel_path), "%s/%s", BASE_DIR, old_channel);
        snprintf(new_channel_path, sizeof(new_channel_path), "%s/%s", BASE_DIR, new_channel);

        if (rename(old_channel_path, new_channel_path) != 0) {
            char response[] = "Error: Could not rename channel directory\n";
            write(client_socket, response, strlen(response));
            return;
        }

        char response[256];
        snprintf(response, sizeof(response), "%s berhasil diubah menjadi %s\n", old_channel, new_channel);
        write(client_socket, response, strlen(response));
    } else {
        remove("channels_temp.csv");
        char response[256];
        snprintf(response, sizeof(response), "Channel %s tidak ditemukan\n", old_channel);
        write(client_socket, response, strlen(response));
    }
}

void handle_delete_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char entry_path[512];
        snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            handle_delete_directory(entry_path);
        } else {
            if (remove(entry_path) != 0) {
                perror("remove");
            }
        }
    }
    closedir(dir);
    if (rmdir(path) != 0) {
        perror("rmdir");
    }
}

void handle_delete_channel(int client_socket, char *channel) {
    char path[256];
    snprintf(path, sizeof(path), "%s/channels.csv", BASE_DIR);
    FILE *file = fopen(path, "r");
    FILE *temp_file = fopen("channels_temp.csv", "w");
    if (!file || !temp_file) {
        char response[] = "Error: Could not open channels.csv or create temporary file\n";
        write(client_socket, response, strlen(response));
        if (file) fclose(file);
        if (temp_file) fclose(temp_file);
        return;
    }

    trim_newline(channel);

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        int id;
        char stored_channel[256], key[256];
        sscanf(line, "%d,%[^,],%s", &id, stored_channel, key);

        // Trim any newline characters from stored_channel and key
        stored_channel[strcspn(stored_channel, "\n")] = 0;
        key[strcspn(key, "\n")] = 0;

        printf("Read channel: %s\n", stored_channel);

        if (strcmp(stored_channel, channel) != 0) {
            fprintf(temp_file, "%d,%s,%s\n", id, stored_channel, key);
        } else {
            found = 1;
        }
    }
    fclose(file);
    fclose(temp_file);

    if (found) {

        if (remove(path) != 0) {
            char response[] = "Error: Could not remove original file\n";
            write(client_socket, response, strlen(response));
            return;
        }
        if (rename("channels_temp.csv", path) != 0) {
            char response[] = "Error: Could not rename temporary file\n";
            write(client_socket, response, strlen(response));
            return;
        }

        // Remove the directory
        char channel_path[512];
        snprintf(channel_path, sizeof(channel_path), "%s/%s", BASE_DIR, channel);
        handle_delete_directory(channel_path);

        char response[256];
        snprintf(response, sizeof(response), "%s berhasil dihapus\n", channel);
        write(client_socket, response, strlen(response));
    } else {
        printf("Channel %s not found in the CSV file\n", channel);
        remove("channels_temp.csv");
        char response[256];
        snprintf(response, sizeof(response), "Channel %s not found\n", channel);
        write(client_socket, response, strlen(response));
    }
}

void handle_create_room(int client_socket, char *channel, char *room) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s/%s", BASE_DIR, channel, room);
    FILE *file;

    // Create room directory
    if (mkdir(path, 0700) == -1) {
        char response[] = "Error: Could not create room directory\n";
        write(client_socket, response, strlen(response));
        return;
    }

    // Create chat file in room directory
    char chat_path[512];
    int chat_path_len = snprintf(chat_path, sizeof(chat_path), "%s/chat.csv", path);
    
    if (chat_path_len >= sizeof(chat_path)) {
        char response[] = "Error: Path is too long for chat.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    file = fopen(chat_path, "w");
    if (!file) {
        char response[] = "Error: Could not create chat.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }
    fclose(file);

    char response[256];
    snprintf(response, sizeof(response), "Room %s created successfully in channel %s\n", room, channel);
    write(client_socket, response, strlen(response));
}

void handle_edit_room(int client_socket, char *channel, char *old_room, char *new_room) {
    char old_room_path[512];
    char new_room_path[512];
    snprintf(old_room_path, sizeof(old_room_path), "%s/%s/%s", BASE_DIR, channel, old_room);
    snprintf(new_room_path, sizeof(new_room_path), "%s/%s/%s", BASE_DIR, channel, new_room);

    if (rename(old_room_path, new_room_path) != 0) {
        char response[] = "Error: Could not rename room directory\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char response[256];
    snprintf(response, sizeof(response), "Room %s in channel %s renamed to %s successfully\n", old_room, channel, new_room);
    write(client_socket, response, strlen(response));
}

void handle_delete_room(int client_socket, char *channel, char *room) {
    char room_path[512];
    snprintf(room_path, sizeof(room_path), "%s/%s/%s", BASE_DIR, channel, room);

    handle_delete_directory(room_path);

    char response[256];
    snprintf(response, sizeof(response), "Room %s in channel %s deleted successfully\n", room, channel);
    write(client_socket, response, strlen(response));
}

void handle_delete_all_rooms(int client_socket) {
    // This function implementation depends on how you structure your room storage.
    // You can iterate through all channels and delete all rooms accordingly.
    char response[] = "All rooms successfully deleted\n";
    write(client_socket, response, strlen(response));
}

void handle_list_channels(int client_socket) {
    char path[256];
    snprintf(path, sizeof(path), "%s/channels.csv", BASE_DIR);
    FILE *file = fopen(path, "r");
    if (!file) {
        char response[] = "Error: Could not open channels.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char line[256];
    char response[1024] = "Channels: ";
    while (fgets(line, sizeof(line), file)) {
        char *id = strtok(line, ",");
        char *channel = strtok(NULL, ",");
        strcat(response,channel);
        strcat(response, " ");
    }
    response[strlen(response) - 1] = '\n'; // Replace the last space with a newline character

    write(client_socket, response, strlen(response));
    fclose(file);
}

void handle_join_channel(int client_socket, char *channel, char *hashed_key) {
    char path[256];
    snprintf(path, sizeof(path), "%s/channels.csv", BASE_DIR);
    FILE *file = fopen(path, "r");
    if (!file) {
        char response[] = "Error: Could not open channels.csv\n";
        write(client_socket, response, strlen(response));
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *id = strtok(line, ",");
        char *channel_name = strtok(NULL, ",");
        char *stored_hashed_key = strtok(NULL, ",");
        if (strcmp(channel_name, channel) == 0) {
            char response[] = "Joined channel successfully\n";
            write(client_socket, response, strlen(response));
            return;
        }
    }

    char response[] = "Invalid channel or key\n";
    write(client_socket, response, strlen(response));
    fclose(file);
}

void handle_join_room(int client_socket, char *channel, char *room) {
    char path[256];
    trim_newline(channel);
    trim_newline(room);
    snprintf(path, sizeof(path), "%s/%s/%s", BASE_DIR, channel, room);
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        char response[] = "Room not found\n";
        write(client_socket, response, strlen(response));
    } else {
        char response[256];
        snprintf(response, sizeof(response), "Joined room successfully\n");
        write(client_socket, response, strlen(response));
    }
}

void handle_send_chat(int client_socket, char *text) {
    char path[256];
    snprintf(path, sizeof(path), "%s/chat.csv", BASE_DIR);
    FILE *file = fopen(path, "a+");
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
    char path[256];
    snprintf(path, sizeof(path), "%s/chat.csv", BASE_DIR);
    FILE *file = fopen(path, "r");
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

void handle_exit(int client_socket, char *channel, char *room) {
    char response[256];

    if (room && strlen(room) > 0) {
        snprintf(response, sizeof(response), "Left room %s successfully\n", room);
    } else if (channel && strlen(channel) > 0) {
        snprintf(response, sizeof(response), "Left channel %s successfully\n", channel);
    } else {
        snprintf(response, sizeof(response), "Logged out successfully\n");
        write(client_socket, response, strlen(response));
        close(client_socket); // Close the socket to exit the application
        exit(0); // Exit the application
    }

    write(client_socket, response, strlen(response));
}

// Handle removing a user from a channel
void handle_remove_user(int client_socket, char *channel, char *username) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s/admin/auth.csv", BASE_DIR, channel);
    FILE *file = fopen(path, "r");
    FILE *temp_file = fopen("auth_temp.csv", "w");
    if (!file || !temp_file) {
        char response[] = "Error: Could not open auth.csv or create temporary file\n";
        write(client_socket, response, strlen(response));
        if (file) fclose(file);
        if (temp_file) fclose(temp_file);
        return;
    }

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        char *id = strtok(line, ",");
        char *stored_username = strtok(NULL, ",");
        char *role = strtok(NULL, ",");

        if (strcmp(stored_username, username) != 0) {
            fprintf(temp_file, "%s,%s,%s\n", id, stored_username, role);
        } else {
            found = 1;
        }
    }
    fclose(file);
    fclose(temp_file);

    if (found) {
        remove(path);
        rename("auth_temp.csv", path);

        char response[256];
        snprintf(response, sizeof(response), "User %s removed from channel %s successfully\n", username, channel);
        write(client_socket, response, strlen(response));
    } else {
        remove("auth_temp.csv");
        char response[256];
        snprintf(response, sizeof(response), "User %s not found in channel %s\n", username, channel);
        write(client_socket, response, strlen(response));
    }
}

// Handle banning a user from a channel
void handle_ban_user(int client_socket, char *channel, char *username) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s/admin/auth.csv", BASE_DIR, channel);
    FILE *file = fopen(path, "r");
    FILE *temp_file = fopen("auth_temp.csv", "w");
    if (!file || !temp_file) {
        char response[] = "Error: Could not open auth.csv or create temporary file\n";
        write(client_socket, response, strlen(response));
        if (file) fclose(file);
        if (temp_file) fclose(temp_file);
        return;
    }

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        char *id = strtok(line, ",");
        char *stored_username = strtok(NULL, ",");
        char *role = strtok(NULL, ",");

        if (strcmp(stored_username, username) == 0) {
            fprintf(temp_file, "%s,%s,BANNED\n", id, stored_username);
            found = 1;
        } else {
            fprintf(temp_file, "%s,%s,%s\n", id, stored_username, role);
        }
    }
    fclose(file);
    fclose(temp_file);

    if (found) {
        remove(path);
        rename("auth_temp.csv", path);

        char response[256];
        snprintf(response, sizeof(response), "User %s banned from channel %s successfully\n", username, channel);
        write(client_socket, response, strlen(response));
    } else {
        remove("auth_temp.csv");
        char response[256];
        snprintf(response, sizeof(response), "User %s not found in channel %s\n", username, channel);
        write(client_socket, response, strlen(response));
    }
}

// Handle unbanning a user from a channel
void handle_unban_user(int client_socket, char *channel, char *username) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s/admin/auth.csv", BASE_DIR, channel);
    FILE *file = fopen(path, "r");
    FILE *temp_file = fopen("auth_temp.csv", "w");
    if (!file || !temp_file) {
        char response[] = "Error: Could not open auth.csv or create temporary file\n";
        write(client_socket, response, strlen(response));
        if (file) fclose(file);
        if (temp_file) fclose(temp_file);
        return;
    }

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        char *id = strtok(line, ",");
        char *stored_username = strtok(NULL, ",");
        char *role = strtok(NULL, ",");

        if (strcmp(stored_username, username) == 0 && strcmp(role, "BANNED") == 0) {
            fprintf(temp_file, "%s,%s,USER\n", id, stored_username);
            found = 1;
        } else {
            fprintf(temp_file, "%s,%s,%s\n", id, stored_username, role);
        }
    }
    fclose(file);
    fclose(temp_file);

    if (found) {
        remove(path);
        rename("auth_temp.csv", path);

        char response[256];
        snprintf(response, sizeof(response), "User %s unbanned from channel %s successfully\n", username, channel);
        write(client_socket, response, strlen(response));
    } else {
        remove("auth_temp.csv");
        char response[256];
        snprintf(response, sizeof(response), "User %s not found or not banned in channel %s\n", username, channel);
        write(client_socket, response, strlen(response));
    }
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
