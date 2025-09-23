# AeroTech-Directory-OA-
Program that compares two directories and detects duplicate files



# Requirements

https://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html

https://www.clonefileschecker.com/blog/hashing-algorithms-to-find-duplicates/
https://docs.openssl.org/3.2/man3/SHA256_Init/#name 
openssl

gcc main.c -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto -o main