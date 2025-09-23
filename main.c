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

HashNode *dir1_hash_list = NULL;
HashNode *dir1_end_node = NULL;
HashNode *dir2_hash_list = NULL;
HashNode *dir2_end_node = NULL;


int read_directory(const char *path, const char *relative_path) {
    char *full_path = malloc(strlen(relative_path) + 1 + strlen(path) + 1);
    sprintf(full_path, "%s/%s", path, relative_path);
    DIR *dir = opendir(full_path);
    free(full_path);

    if (dir == NULL) {
        printf("No Directory found at: %s/%s\n", path, relative_path);
        return -1;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' || // Skip "."
            (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))){ // Skip ".."
            continue;
        }

        else if(entry->d_type == DT_DIR){ // If entry is a directory get that directory's contents
            char *new_relative_path = malloc(strlen(relative_path) + 1 + strlen(entry->d_name) + 1);
            sprintf(new_relative_path, "%s/%s", relative_path, entry->d_name);
            printf("%s/\n", new_relative_path);
            read_directory(path, new_relative_path);
            free(new_relative_path);
        }   

        else{ // If entry is a file print its name
            printf("%s/%s\n", relative_path, entry->d_name);
            
            // compute its sha256 hash
            char *full_file_path = malloc(strlen(relative_path) + 1 + strlen(path) + 1 + strlen(entry->d_name) + 1);
            if (strlen(relative_path) == 0)
                sprintf(full_file_path, "%s/%s", path, entry->d_name);
            else
                sprintf(full_file_path, "%s/%s/%s", path, relative_path, entry->d_name);
            unsigned char file_hash[SHA256_DIGEST_LENGTH];
            
            if (compute_sha256(full_file_path, file_hash) != 1 ){
                printf("Error computing SHA-256 for file: %s\n", full_file_path);
            }
            else{
                // Check if Hash already exists in the list
                HashNode* temp = dir1_hash_list;
                while (temp != NULL) {
                    if (memcmp(temp->hash, file_hash, SHA256_DIGEST_LENGTH) == 0) {
                        printf("Duplicate found: %s and %s\n", temp->filepath, full_file_path);
                        break;
                    }
                    temp = temp->next;
                }

                if (temp == NULL){ // No duplicate found, add the hash to list
                    HashNode* new_node = create_hash_node(file_hash, full_file_path);

                    if (dir1_hash_list == NULL) {
                        dir1_hash_list = new_node;
                        dir1_end_node = new_node;
                    } else {
                        dir1_end_node->next = new_node;
                        dir1_end_node = new_node;
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


int main() {
    char path[256];
    printf("Enter a file path: ");
    scanf("%255s", path);

    char relative_path[] = "";
    if (read_directory(path, relative_path) != 1) {
        perror("Error reading directory");
        return -1;
    }

    print_hash_list(dir1_hash_list);

    free_hash_list(dir1_hash_list);
    dir1_end_node = NULL;
    free_hash_list(dir2_hash_list);
    dir2_end_node = NULL;
}