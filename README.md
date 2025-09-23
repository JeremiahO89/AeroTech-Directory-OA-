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
Creates a new linked list node storing a file’s SHA-256 hash and its file path.  

---

### `void free_hash_list(HashNode *head)`  
Frees the memory of an entire hash linked list, including file path strings.  

---

### `int compute_sha256(const char *filepath, unsigned char hash[SHA256_DIGEST_LENGTH])`  
Computes the SHA-256 hash of a given file. Reads the file in 8KB chunks. Returns `1` on success, `-1` on failure.  

---

### `int read_directory(const char *path, const char *relative_path, HashNode **head, HashNode **tail)`  
Recursively reads a directory, computes SHA-256 hashes of all files, and adds them to a linked list. Skips duplicate hashes within the same directory list.  

- `path`: Root directory  
- `relative_path`: Relative subdirectory path during recursion  
- `head`/`tail`: Pointers to the linked list of file hashes  

---

### `void print_hash_list(HashNode *head)`  
Prints all hashes and file paths in a linked list. Shows the hash in hexadecimal format.  

---

### `void field_functions(const char *command, HashNode *dir1_hash_list, HashNode *dir2_hash_list)`  
Performs operations based on the user’s command:  

- `display`: Lists files in the first directory that have the same hash as files in the second directory  
- `delete`: Deletes files in the first directory that are also present in the second directory  
- `copy`: Lists files in the first directory that do not exist in the second directory (copy logic can be added later)  

---

### `int main()`  
- Prompts the user for a command and two directory paths  
- Builds SHA-256 hash linked lists for each directory  
- Calls `field_functions` to perform the requested operation  
- Frees memory for the hash lists  

---

## References

- [POSIX `dirent.h`](https://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html) – Documentation for reading directories in C  
- [Hashing Algorithms to Detect Duplicate Files](https://www.clonefileschecker.com/blog/hashing-algorithms-to-find-duplicates/) – Idea to use hashes instead of byte-by-byte comparisons  
- [ChatGPT](https://chat.openai.com/) – Guidance on commands, implementation ideas, and Markdown formatting  
