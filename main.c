#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>


int read_directory(const char *path, const char *relative_path) {
    
    char *fullpath = malloc(strlen(relative_path) + 1 + strlen(path) + 1);
    sprintf(fullpath, "%s/%s", path, relative_path);
    DIR *dir = opendir(fullpath);
    free(fullpath);

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
        }
        
    } 
    closedir(dir);
    return 1;
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

}