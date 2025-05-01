#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structures
typedef struct File {
    char name[100];
    struct File* next;
} File;

typedef struct Directory {
    char name[100];
    struct Directory* parent;
    struct Directory* child;
    struct Directory* sibling;
    File* files;
} Directory;

Directory* root;
Directory* current;

// Utility Functions
Directory* createDirectory(const char* name) {
    Directory* newDir = (Directory*)malloc(sizeof(Directory));
    strcpy(newDir->name, name);
    newDir->parent = NULL;
    newDir->child = NULL;
    newDir->sibling = NULL;
    newDir->files = NULL;
    return newDir;
}

File* createFile(const char* name) {
    File* newFile = (File*)malloc(sizeof(File));
    strcpy(newFile->name, name);
    newFile->next = NULL;
    return newFile;
}

// Command Implementations
void mkdir(const char* name) {
    Directory* newDir = createDirectory(name);
    newDir->parent = current;

    if (current->child == NULL) {
        current->child = newDir;
    } else {
        Directory* temp = current->child;
        while (temp->sibling != NULL) {
            if (strcmp(temp->name, name) == 0) {
                printf("Directory already exists!\n");
                free(newDir);
                return;
            }
            temp = temp->sibling;
        }
        if (strcmp(temp->name, name) == 0) {
            printf("Directory already exists!\n");
            free(newDir);
            return;
        }
        temp->sibling = newDir;
    }
}

void touch(const char* name) {
    File* newFile = createFile(name);

    if (current->files == NULL) {
        current->files = newFile;
    } else {
        File* temp = current->files;
        while (temp->next != NULL) {
            if (strcmp(temp->name, name) == 0) {
                printf("File already exists!\n");
                free(newFile);
                return;
            }
            temp = temp->next;
        }
        if (strcmp(temp->name, name) == 0) {
            printf("File already exists!\n");
            free(newFile);
            return;
        }
        temp->next = newFile;
    }
}

void cd(const char* name) {
    if (strcmp(name, "..") == 0) {
        if (current->parent != NULL)
            current = current->parent;
        return;
    }

    Directory* temp = current->child;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            current = temp;
            return;
        }
        temp = temp->sibling;
    }

    printf("Directory not found!\n");
}

void ls() {
    Directory* tempDir = current->child;
    File* tempFile = current->files;

    while (tempDir != NULL) {
        printf("[D] %s\n", tempDir->name);
        tempDir = tempDir->sibling;
    }

    while (tempFile != NULL) {
        printf("[F] %s\n", tempFile->name);
        tempFile = tempFile->next;
    }
}

void pwd() {
    if (current == root) {
        printf("/\n");
        return;
    }

    char path[1000] = "";
    Directory* temp = current;
    while (temp != NULL) {
        char buffer[100];
        sprintf(buffer, "/%s", temp->name);
        strcat(buffer, path);
        strcpy(path, buffer);
        temp = temp->parent;
    }

    printf("%s\n", path);
}

void rm(const char* name) {
    File *prev = NULL, *temp = current->files;

    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            if (prev == NULL) {
                current->files = temp->next;
            } else {
                prev->next = temp->next;
            }
            free(temp);
            printf("File deleted successfully.\n");
            return;
        }
        prev = temp;
        temp = temp->next;
    }

    printf("File not found!\n");
}

void rmdir(const char* name) {
    Directory *prev = NULL, *temp = current->child;

    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            if (temp->child != NULL || temp->files != NULL) {
                printf("Directory is not empty!\n");
                return;
            }

            if (prev == NULL) {
                current->child = temp->sibling;
            } else {
                prev->sibling = temp->sibling;
            }

            free(temp);
            printf("Directory deleted successfully.\n");
            return;
        }
        prev = temp;
        temp = temp->sibling;
    }

    printf("Directory not found!\n");
}

// Command Parser
void executeCommand(char* input) {
    char* command = strtok(input, " \n");
    char* arg = strtok(NULL, " \n");

    if (command == NULL) return;

    if (strcmp(command, "mkdir") == 0 && arg) {
        mkdir(arg);
    } else if (strcmp(command, "touch") == 0 && arg) {
        touch(arg);
    } else if (strcmp(command, "cd") == 0 && arg) {
        cd(arg);
    } else if (strcmp(command, "ls") == 0) {
        ls();
    } else if (strcmp(command, "pwd") == 0) {
        pwd();
    } else if (strcmp(command, "rm") == 0 && arg) {
        rm(arg);
    } else if (strcmp(command, "rmdir") == 0 && arg) {
        rmdir(arg);
    } else if (strcmp(command, "exit") == 0) {
        exit(0);
    } else {
        printf("Invalid command or missing argument.\n");
    }
}

// Main Function
int main() {
    root = createDirectory("root");
    current = root;

    char input[200];
    printf("Welcome to Custom File System! Type 'exit' to quit.\n");

    while (1) {
        printf(">> ");
        fgets(input, sizeof(input), stdin);
        executeCommand(input);
    }

    return 0;
}
