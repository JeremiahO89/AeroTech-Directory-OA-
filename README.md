# AeroTech-Directory-OA-

A program that compares two directories, computes SHA-256 hashes of their files, and detects duplicate files. It supports displaying duplicates, deleting them, or copying unique files to a new location.  

> **Note:** The file copying feature is not fully implemented. In C, copying files requires creating a new file, reading the original file, and writing its contents to the new file. I ran out of time to implement this but may revisit it later.  

This assignment took about 3.5 hours from start to finish. Some of that time was spent on setup since I’m using macOS this semester instead of Windows, and I ran into a 45-minute issue getting OpenSSL to work with VS Code. I took my time on the assignment and didn’t rely on generative AI because I found the prompt interesting and wanted to understand how to solve it. I haven’t programmed in C for a while—mostly Python recently—so it took me some time to get back into it. I also researched different approaches to make sure I implemented it properly, such as using hashing for the files. I considered alternatives to the linked list, knowing it could be slow with large datasets, but other approaches seemed too complex to implement within the available time, and I didn't fully understand them from briefly skimming the articles.  

I performed basic testing on my computer using test directories. One limitation I found is that directory names cannot contain spaces. This is because `scanf` reads input until the first space, which breaks paths containing spaces. This is a known bug that could be addressed in the future.  

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

**Note:** The first directory entered is the one that will be affected by the operation. For example, if duplicates exist, files in the first directory will be deleted or listed, depending on the command.

- **display**: Shows duplicate files in both directories  
- **delete**: Deletes duplicate files from the first directory  
- **copy**: Lists files in the first directory that do not exist in the second directory (copy logic can be added later)  

---

## Function Descriptions

### `HashNode* create_hash_node(unsigned char hash[SHA256_DIGEST_LENGTH], const char *filepath)`  
**Functionality:**  
Creates a new linked list node to store a file’s SHA-256 hash and its file path.  

**Arguments:**  
- `hash`: SHA-256 hash of the file (32 bytes)  
- `filepath`: Path to the file  

**Results:**  
Returns a pointer to the newly created `HashNode`. Returns `NULL` if memory allocation fails.  

---

### `void free_hash_list(HashNode *head)`  
**Functionality:**  
Frees memory allocated for a linked list of hash nodes.  

**Arguments:**  
- `head`: Pointer to the head of the linked list  

**Results:**  
All nodes and their associated file path strings are freed. No return value.  

---

### `int compute_sha256(const char *filepath, unsigned char hash[SHA256_DIGEST_LENGTH])`  
**Functionality:**  
Computes the SHA-256 hash of a file by reading it in 8KB chunks.  

**Arguments:**  
- `filepath`: Path to the file to hash  
- `hash`: Buffer to store the resulting SHA-256 hash  

**Results:**  
Returns `1` on success, `-1` if the file cannot be opened.  

---

### `int read_directory(const char *path, const char *relative_path, HashNode **head, HashNode **tail)`  
**Functionality:**  
Recursively reads all files in a directory and subdirectories, computes SHA-256 hashes, and adds unique hashes to a linked list.  

**Arguments:**  
- `path`: Root directory path  
- `relative_path`: Current relative path during recursion  
- `head`: Pointer to the head of the hash list  
- `tail`: Pointer to the tail of the hash list  

**Results:**  
Returns `1` on success, `-1` if the directory cannot be opened. Populates the linked list with unique file hashes.  

---

### `void print_hash_list(HashNode *head)`  
**Functionality:**  
Prints all nodes in a hash list, including SHA-256 hashes and file paths.  

**Arguments:**  
- `head`: Pointer to the head of the linked list  

**Results:**  
Displays each file’s hash in hexadecimal and its path. Prints a message if the list is empty.  

---

### `void field_functions(const char *command, HashNode *dir1_hash_list, HashNode *dir2_hash_list)`  
**Functionality:**  
Performs operations (`display`, `delete`, or `copy`) on two directories based on file hashes.  

**Arguments:**  
- `command`: User-specified operation (`display`, `delete`, or `copy`)  
- `dir1_hash_list`: Hash list of the first directory  
- `dir2_hash_list`: Hash list of the second directory  

**Results:**  
- `display`: Prints files in directory 1 that are duplicates in directory 2  
- `delete`: Deletes files in directory 1 that exist in directory 2  
- `copy`: Prints files in directory 1 that are not in directory 2 (copying not yet implemented)  

---

### `int main()`  
**Functionality:**  
Main program flow. Prompts the user for input, builds hash lists for two directories, performs the selected operation, and frees memory.  

**Arguments:**  
None  

**Results:**  
Executes the program workflow and returns `0` on success, `-1` on error reading directories.  

## References

- [POSIX `dirent.h`](https://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html) – Documentation for reading directories in C  
- [Hashing Algorithms to Detect Duplicate Files](https://www.clonefileschecker.com/blog/hashing-algorithms-to-find-duplicates/) – Idea to use hashes instead of byte-by-byte comparisons  
- [ChatGPT](https://chat.openai.com/) – Guidance on commands, implementation ideas, and Markdown formatting  
