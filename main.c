#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <openssl/sha.h>

typedef struct HashNode {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char *filepath;
    struct HashNode *next;
} HashNode;

HashNode* create_hash_node(unsigned char hash[SHA256_DIGEST_LENGTH], const char *filepath) {
    HashNode *new_node = malloc(sizeof(HashNode));
    memcpy(new_node->hash, hash, SHA256_DIGEST_LENGTH);

    new_node->filepath = malloc(strlen(filepath) + 1);
    if (!new_node->filepath) {
        free(new_node);
        return NULL;
    }
    memcpy(new_node->filepath, filepath, strlen(filepath) + 1);
    
    new_node->next = NULL;
    return new_node; 
}

void free_hash_list(HashNode *head) {
    while (head) {
        HashNode *tmp = head;
        head = head->next;
        free(tmp->filepath);
        free(tmp);
    }
}

int compute_sha256(const char *filepath, unsigned char hash[SHA256_DIGEST_LENGTH]) {
    FILE *file = fopen(filepath, "rb");
    if (!file) return -1;

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    unsigned char buffer[8192]; // read in 8KB chunks
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        SHA256_Update(&sha256, buffer, bytesRead);
    }
    SHA256_Final(hash, &sha256);
    fclose(file);
    return 1;
}

int read_directory(const char *path, const char *relative_path, HashNode **head, HashNode **tail) {
    char *full_path = malloc(strlen(path) + 1 + strlen(relative_path) + 1);
    if (strlen(relative_path) == 0)
        sprintf(full_path, "%s", path);
    else
        sprintf(full_path, "%s/%s", path, relative_path);

    DIR *dir = opendir(full_path);
    free(full_path);

    if (!dir) {
        printf("No Directory found at: %s/%s\n", path, relative_path);
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' || // Skip "."
            (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) { // Skip ".."
            continue;
        }
        else if(entry->d_type == DT_DIR) { // If entry is a directory get that directory's contents
            char *new_relative_path = malloc(strlen(relative_path) + 1 + strlen(entry->d_name) + 1);
            if (strlen(relative_path) == 0)
                sprintf(new_relative_path, "%s", entry->d_name);
            else
                sprintf(new_relative_path, "%s/%s", relative_path, entry->d_name);

            read_directory(path, new_relative_path, head, tail);
            free(new_relative_path);
        } 
        else {
            char *full_file_path;
            if (strlen(relative_path) == 0)
                full_file_path = malloc(strlen(path) + 1 + strlen(entry->d_name) + 1);
            else
                full_file_path = malloc(strlen(path) + 1 + strlen(relative_path) + 1 + strlen(entry->d_name) + 1);

            if (strlen(relative_path) == 0)
                sprintf(full_file_path, "%s/%s", path, entry->d_name);
            else
                sprintf(full_file_path, "%s/%s/%s", path, relative_path, entry->d_name);

            unsigned char file_hash[SHA256_DIGEST_LENGTH];
            if (compute_sha256(full_file_path, file_hash) == 1) {
                // check duplicates in this list
                HashNode *temp = *head;
                int duplicate = 0;
                while (temp) {
                    if (memcmp(temp->hash, file_hash, SHA256_DIGEST_LENGTH) == 0) {
                        duplicate = 1;
                        break;
                    }
                    temp = temp->next;
                }

                if (!duplicate) {
                    HashNode *new_node = create_hash_node(file_hash, full_file_path);
                    if (!*head) {
                        *head = *tail = new_node;
                    } else {
                        (*tail)->next = new_node;
                        *tail = new_node;
                    }
                }
            }
            free(full_file_path);
        }
    }
    closedir(dir);
    return 1;
}

void print_hash_list(HashNode *head) {
    if (!head) {
        printf("Hash list is empty.\n");
        return;
    }

    HashNode *current = head;
    while (current) {
        // Print SHA-256 hash as hex
        printf("Hash: ");
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            printf("%02x", current->hash[i]);
        }
        // Print file path
        printf("  File: %s\n", current->filepath);
        current = current->next;
    }
}

void field_functions(const char *command, HashNode *dir1_hash_list, HashNode *dir2_hash_list){

    if (strcmp(command, "display") == 0) {
        // display files in list 1 that are also in list 2
        HashNode *temp1 = dir1_hash_list;
        while (temp1) {
            HashNode *temp2 = dir2_hash_list;
            while (temp2) {
                if (memcmp(temp1->hash, temp2->hash, SHA256_DIGEST_LENGTH) == 0) {
                    printf("Hash: ");
                    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                        printf("%02x", temp1->hash[i]);
                    }
                    // Print file path
                    printf("  File: %s\n", temp1->filepath);
                }
                temp2 = temp2->next; // advance inner loop
            }
            temp1 = temp1->next; // advance outer loop
        }
    } 
    else if (strcmp(command, "delete") == 0) {
        // delete files in list 1 that are also in list 2
        HashNode *temp1 = dir1_hash_list;
        while (temp1) {
            HashNode *temp2 = dir2_hash_list;
            while (temp2) {
                if (memcmp(temp1->hash, temp2->hash, SHA256_DIGEST_LENGTH) == 0) {
                    if (remove(temp1->filepath) == 0) {
                        printf("Deleted: %s\n", temp1->filepath);
                    } else {
                        printf("Error Deleating: %s", temp1->filepath);
                    }
                    break;
                }
                temp2 = temp2->next;
            }
            temp1 = temp1->next;
        }
    } 
    else if (strcmp(command, "copy") == 0) {
        // copy files in list 1 that are not in list 2 to a new directory
        HashNode *temp1 = dir1_hash_list;
        while (temp1) {
            int found = 0;
            HashNode *temp2 = dir2_hash_list;
            while (temp2) {
                if (memcmp(temp1->hash, temp2->hash, SHA256_DIGEST_LENGTH) == 0) {
                    found = 1;
                    break;
                }
                temp2 = temp2->next;
            }
            if (!found) {
                printf("Would copy: %s\n", temp1->filepath);
                // Implement copy logic here
            }
            temp1 = temp1->next;
        }
    } 
    else {
        printf("Invalid command\n");
    }

}

int main() {
    HashNode *dir1_hash_list = NULL;
    HashNode *dir1_end_node = NULL;
    HashNode *dir2_hash_list = NULL;
    HashNode *dir2_end_node = NULL;

    // Get user input for command and directory paths
    char command[8];
    printf("Enter command (display, delete, or copy): ");
    scanf("%7s", command);

    char path[256];
    printf("Enter a file path: ");
    scanf("%255s", path);

    char relative_path[] = "";
    if (read_directory(path, relative_path, &dir1_hash_list, &dir1_end_node) != 1) {
        perror("Error reading directory");
        return -1;
    }

    printf("Enter seccond file path: ");
    scanf("%255s", path);

    if (read_directory(path, relative_path, &dir2_hash_list, &dir2_end_node) != 1) {
        perror("Error reading directory");
        return -1;
    }

    // Preform the desired user command
    printf("\n");
    field_functions(command, dir1_hash_list, dir2_hash_list);

    
    // tests to verify hash lists
    // print_hash_list(dir1_hash_list);
    // print_hash_list(dir2_hash_list);


    // Free allocated memory
    free_hash_list(dir1_hash_list);
    dir1_end_node = NULL;
    free_hash_list(dir2_hash_list);
    dir2_end_node = NULL;
}