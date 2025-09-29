#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <openssl/sha.h>

// Linked list node to store file hash and path
typedef struct HashNode {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char *filepath;
    struct HashNode *next;
} HashNode;

// Create a new hash node
HashNode* create_hash_node(unsigned char hash[SHA256_DIGEST_LENGTH], const char *filepath) {
    HashNode *new_node = malloc(sizeof(HashNode));
    memcpy(new_node->hash, hash, SHA256_DIGEST_LENGTH);

    // Copy filepath string
    new_node->filepath = malloc(strlen(filepath) + 1);
    if (!new_node->filepath) {
        free(new_node); // clean up if malloc fails
        return NULL;
    }
    memcpy(new_node->filepath, filepath, strlen(filepath) + 1);
    
    new_node->next = NULL; // end of list for now
    return new_node; 
}

// Free the linked list
void free_hash_list(HashNode *head) {
    while (head) {
        HashNode *tmp = head;
        head = head->next;
        free(tmp->filepath);
        free(tmp);
    }
}

// Compute SHA-256 hash for a given file
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

// Recursively read directory and hash all files
int read_directory(const char *path, const char *relative_path, HashNode **head, HashNode **tail) {
    // Make and load the full_dir_path
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

    // Iterate through directory entries
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // skip . and ..
        if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' || 
            (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
            continue;
        }
        else if(entry->d_type == DT_DIR) { // recurse into subdirectory
            char *new_relative_path = malloc(strlen(relative_path) + 1 + strlen(entry->d_name) + 1);
            if (strlen(relative_path) == 0)
                sprintf(new_relative_path, "%s", entry->d_name);
            else
                sprintf(new_relative_path, "%s/%s", relative_path, entry->d_name);

            read_directory(path, new_relative_path, head, tail);
            free(new_relative_path);
        } 
        else { // regular file - compute hash
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
                // check if duplicates already in list
                HashNode *temp = *head;
                int duplicate = 0;
                while (temp) {
                    if (memcmp(temp->hash, file_hash, SHA256_DIGEST_LENGTH) == 0) {
                        duplicate = 1;
                        break;
                    }
                    temp = temp->next;
                }

                // add to node list if not duplicate
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

// Print function for debugging
void print_hash_list(HashNode *head) {
    if (!head) {
        printf("Hash list is empty.\n");
        return;
    }

    HashNode *current = head;
    while (current) {
        // print hash as hex
        printf("Hash: ");
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            printf("%02x", current->hash[i]);
        }
        printf("  File: %s\n", current->filepath);
        current = current->next;
    }
}

//Copy file from src to dest
int copy_file(const char *src, const char *dest){
    FILE *src_file = fopen(src, "rb");
    FILE *dest_file = fopen(dest, "wb");
    if (!src_file || !dest_file) {
        if (src_file) fclose(src_file);
        if (dest_file) fclose(dest_file);
        return -1;
    }
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        fwrite(buffer, 1, bytes, dest_file);
    }

    fclose(src_file);
    fclose(dest_file);

    printf("Copied file from %s\n", src);
    printf("To %s\n------------------------------------\n", dest);

    return 1;
}

int make_path_and_copy_file(const char *src, const char *dest_root){
    // Extract filename from src
    char *file_name = strrchr(src, '/');
    if (!file_name) {
        file_name = (char *)src; // no directory part
    } else {
        file_name++; // move past the '/'
    }
    // Create full destination path
    char *dest_path = malloc(strlen(dest_root) + 1 + strlen(file_name) + 1);
    if (!dest_path) return -1; // malloc failed
    sprintf(dest_path, "%s/%s", dest_root, file_name);
    
    // Copy file
    if (copy_file(src, dest_path) != 1) {
        printf("Error copying file to: %s\n", dest_path);
        free(dest_path); 
        return -1;
    }
    free(dest_path);        
    return 1;
}


// Perform command: display, delete, copy
void field_functions(const char *command, HashNode *dir1_hash_list, HashNode *dir2_hash_list, const char *dir2_path) {

    if (strcmp(command, "display") == 0) {
        // display duplicates
        HashNode *temp1 = dir1_hash_list;
        while (temp1) {
            HashNode *temp2 = dir2_hash_list;
            while (temp2) {
                if (memcmp(temp1->hash, temp2->hash, SHA256_DIGEST_LENGTH) == 0) {
                    printf("Hash: ");
                    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                        printf("%02x", temp1->hash[i]);
                    }
                    printf("  File: %s\n", temp1->filepath);
                }
                temp2 = temp2->next;
            }
            temp1 = temp1->next;
        }
    } 
    else if (strcmp(command, "delete") == 0) {
        // remove duplicates from dir1
        HashNode *temp1 = dir1_hash_list;
        while (temp1) {
            HashNode *temp2 = dir2_hash_list;
            while (temp2) {
                if (memcmp(temp1->hash, temp2->hash, SHA256_DIGEST_LENGTH) == 0) {
                    if (remove(temp1->filepath) == 0) {
                        printf("Deleted: %s\n", temp1->filepath);
                    } else {
                        printf("Error Deleting: %s", temp1->filepath);
                    }
                    break;
                }
                temp2 = temp2->next;
            }
            temp1 = temp1->next;
        }
    } 
    else if (strcmp(command, "copy") == 0) {
        // list files that would be copied from dir1 to dir2
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
                // printf("Would copy: %s\n", temp1->filepath);
                // actual copy not implemented yet
                if (!make_path_and_copy_file(temp1->filepath, dir2_path)) {
                    printf("Error creating file path for: %s\n", temp1->filepath);
                }
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

    // Get user input
    char command[8];
    printf("Enter command (display, delete, or copy): ");
    scanf("%7s", command);

    char path[256];
    printf("Enter first file path: ");
    scanf("%255s", path);

    char relative_path[] = "";
    if (read_directory(path, relative_path, &dir1_hash_list, &dir1_end_node) != 1) {
        perror("Error reading directory");
        return -1;
    }

    printf("Enter second file path: ");
    scanf("%255s", path);

    if (read_directory(path, relative_path, &dir2_hash_list, &dir2_end_node) != 1) {
        perror("Error reading directory");
        return -1;
    }

    // Perform command
    printf("\n");
    field_functions(command, dir1_hash_list, dir2_hash_list, path);

    // free memory
    free_hash_list(dir1_hash_list);
    dir1_end_node = NULL;
    free_hash_list(dir2_hash_list);
    dir2_end_node = NULL;
}