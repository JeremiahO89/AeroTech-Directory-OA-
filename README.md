# AeroTech-Directory-OA

A program that compares two directories, computes SHA-256 hashes of their files, and detects duplicate files. It supports displaying duplicates, deleting them, or copying unique files to a new location.  

This assignment took about 5 hours to complete over 2 sessions. Some of that time was spent on setup since I’m using macOS this semester instead of Windows, and I encountered a 45-minute issue getting OpenSSL to work with VS Code. I took my time on the assignment and didn’t rely on generative AI because I found the prompt interesting and wanted to fully understand how to solve it. I haven’t programmed in C for a while—mostly Python recently—so it took some time to get back into it. I also researched different approaches to ensure proper implementation, such as using hashing for files. I considered alternatives to the linked list, knowing it could be slow with large datasets, but other approaches seemed too complex to implement within the available time, and I didn’t fully understand them from briefly skimming the articles.  

I performed basic testing on my computer using test directories to test for various edge cases.  

### Version 2 Improvements

- Directory names can now contain spaces. Previously, `scanf` was used to read user input, which stopped reading at the first space, breaking paths with spaces. This is now resolved by using `fgets` and trimming the trailing newline.  
- The `copy` functionality is implemented. It replicates the directory structure from the source in the destination so new non-duplicate files can be copied.  
- Handles copying files with the same name but different contents by appending `_copy` to the filename.  

---

## System Requirements

- **C Standard Library** (`stdio.h`, `stdlib.h`, `string.h`, `dirent.h`)  
- **OpenSSL Library** for SHA-256 hashing  
  - [OpenSSL SHA256 Documentation](https://docs.openssl.org/3.2/man3/SHA256_Init/#name)  
- Compiler: `gcc` with OpenSSL includes and libraries  

Example compilation commands for macOS:

```bash
gcc main.c -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto -o main
gcc -Wno-deprecated-declarations -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto main.c -o main
```

> **Note:** OpenSSL is deprecated, which generates warnings during compilation. The second `gcc` command suppresses these warnings.  

---

## How to Use

1. Run the program:  

```bash
./main
```

2. Enter the command: `display`, `delete`, or `copy`.  
3. Enter the paths of the two directories to compare.  

- **display**: Shows duplicate files in the first directory that also exist in the second directory.  
- **delete**: Deletes duplicate files from the first directory that exist in the second directory.  
- **copy**: Copies files in the first directory that do not exist in the second directory to the second directory.  

---

## Function Outlines

### `HashNode* create_hash_node(unsigned char hash[SHA256_DIGEST_LENGTH], const char *filepath)`  
- **Purpose:** Create a new linked list node to store a file’s SHA-256 hash and file path.  
- **Input:** `hash` (SHA-256 hash), `filepath` (file path string)  
- **Output:** Pointer to the new `HashNode`, or `NULL` if memory allocation fails  

---

### `void free_hash_list(HashNode *head)`  
- **Purpose:** Free memory for a linked list of hash nodes.  
- **Input:** `head` (pointer to the head node)  
- **Output:** None  

---

### `int compute_sha256(const char *filepath, unsigned char hash[SHA256_DIGEST_LENGTH])`  
- **Purpose:** Compute the SHA-256 hash of a file.  
- **Input:** `filepath` (path to file), `hash` (buffer to store SHA-256 result)  
- **Output:** Returns `1` on success, `-1` if the file cannot be opened  

---

### `int read_directory(const char *path, HashNode **head, HashNode **tail)`  
- **Purpose:** Recursively read all files in a directory and subdirectories, compute SHA-256 hashes, and add unique hashes to a linked list.  
- **Input:** `path` (directory path), `head`/`tail` (pointers to linked list)  
- **Output:** Returns `1` on success, `-1` if directory cannot be opened  

---

### `void print_hash_list(HashNode *head)`  
- **Purpose:** Print all nodes in a hash list, including hash and file path.  
- **Input:** `head` (pointer to linked list head)  
- **Output:** Prints each file hash in hexadecimal and the path  

---

### `int copy_file(const char *src, const char *dest)`  
- **Purpose:** Copy a file from source to destination. Adds `_copy` to filename if destination already exists.  
- **Input:** `src` (source file), `dest` (destination path)  
- **Output:** Returns `1` on success, `-1` on error  

---

### `int dir_exists(const char *path)`  
- **Purpose:** Check if a directory exists.  
- **Input:** `path` (directory path)  
- **Output:** Returns `1` if directory exists, `0` otherwise  

---

### `int make_path_and_copy_file(const char *src, const char *src_root, const char *dest_root)`  
- **Purpose:** Create necessary directories in the destination and copy the source file while preserving the folder structure.  
- **Input:** `src` (source file path), `src_root` (root of source directory), `dest_root` (destination root)  
- **Output:** Returns `1` on success, `-1` on failure  

---

### `void field_functions(const char *command, HashNode *dir1_hash_list, HashNode *dir2_hash_list, const char *dir1_path, const char *dir2_path)`  
- **Purpose:** Perform `display`, `delete`, or `copy` commands on two directories based on SHA-256 hashes.  
- **Input:**  
  - `command` (user-specified command)  
  - `dir1_hash_list`/`dir2_hash_list` (hash lists)  
  - `dir1_path`/`dir2_path` (directory paths for copying)  
- **Output:** Executes the chosen command and prints results  

---

### `int main()`  
- **Purpose:** Program entry point. Handles user input, builds hash lists, performs operations, and frees memory.  
- **Input:** None (prompts user)  
- **Output:** Returns `0` on success, `-1` on error  

---

## References

- [POSIX `dirent.h`](https://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html) – Documentation for reading directories in C  
- [Hashing Algorithms to Detect Duplicate Files](https://www.clonefileschecker.com/blog/hashing-algorithms-to-find-duplicates/) – Using hashes to detect duplicates  
- [ChatGPT](https://chat.openai.com/) – Guidance on commands, implementation ideas, and Markdown formatting  
