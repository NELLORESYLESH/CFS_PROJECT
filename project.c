#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct File {
    char name[100];
    char content[1000];
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

// --------- Directory and File Management ---------
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
    newFile->content[0] = '\0';
    newFile->next = NULL;
    return newFile;
}

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
        printf("/root\n");
        return;
    }

    char* names[100];
    int depth = 0;
    Directory* temp = current;

    while (temp != NULL && temp != root) {
        names[depth++] = temp->name;
        temp = temp->parent;
    }

    printf("/root");
    for (int i = depth - 1; i >= 0; i--) {
        printf("/%s", names[i]);
    }
    printf("\n");
}

void rm(const char* name) {
    File *prev = NULL, *temp = current->files;

    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            if (prev == NULL)
                current->files = temp->next;
            else
                prev->next = temp->next;
            free(temp);
            printf("File deleted.\n");
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

            if (prev == NULL)
                current->child = temp->sibling;
            else
                prev->sibling = temp->sibling;

            free(temp);
            printf("Directory deleted.\n");
            return;
        }
        prev = temp;
        temp = temp->sibling;
    }

    printf("Directory not found!\n");
}

File* findFile(const char* name) {
    File* temp = current->files;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}

void edit(const char* filename) {
    File* file = findFile(filename);
    if (!file) {
        printf("File not found!\n");
        return;
    }

    printf("Enter content (type '::end' to finish):\n");
    file->content[0] = '\0';
    char line[256];
    while (1) {
        fgets(line, sizeof(line), stdin);
        if (strncmp(line, "::end", 5) == 0) break;
        strcat(file->content, line);
    }
    printf("Content saved.\n");
}

void view(const char* filename) {
    File* file = findFile(filename);
    if (!file) {
        printf("File not found!\n");
        return;
    }
    printf("--- %s ---\n%s\n", file->name, file->content[0] ? file->content : "[Empty File]");
}

void exportFile(const char* filename) {
    File* file = findFile(filename);
    if (!file) {
        printf("File not found in virtual FS.\n");
        return;
    }

    FILE* f = fopen(file->name, "w");
    if (!f) {
        printf("Error exporting file.\n");
        return;
    }

    fprintf(f, "%s", file->content);
    fclose(f);
    printf("Exported to disk.\n");
}

void importFile(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("File not found on disk.\n");
        return;
    }

    touch(filename);
    File* file = findFile(filename);
    if (!file) {
        printf("Could not create virtual file.\n");
        fclose(f);
        return;
    }

    file->content[0] = '\0';
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        strcat(file->content, line);
    }

    fclose(f);
    printf("Imported from disk.\n");
}

// --------- Command Execution ---------
void executeCommand(char* input) {
    char* command = strtok(input, " \n");
    char* arg = strtok(NULL, " \n");

    if (command == NULL) return;

    if (strcmp(command, "mkdir") == 0 && arg)
        mkdir(arg);
    else if (strcmp(command, "touch") == 0 && arg)
        touch(arg);
    else if (strcmp(command, "cd") == 0 && arg)
        cd(arg);
    else if (strcmp(command, "ls") == 0)
        ls();
    else if (strcmp(command, "pwd") == 0)
        pwd();
    else if (strcmp(command, "rm") == 0 && arg)
        rm(arg);
    else if (strcmp(command, "rmdir") == 0 && arg)
        rmdir(arg);
    else if (strcmp(command, "edit") == 0 && arg)
        edit(arg);
    else if (strcmp(command, "view") == 0 && arg)
        view(arg);
    else if (strcmp(command, "export") == 0 && arg)
        exportFile(arg);
    else if (strcmp(command, "import") == 0 && arg)
        importFile(arg);
    else if (strcmp(command, "exit") == 0)
        exit(0);
    else
        printf("Invalid command or missing argument.\n");
}

// --------- Main Program with Dynamic Prompt ---------
int main() {
    root = createDirectory("root");
    current = root;

    char input[200];
    printf("Welcome to Custom File System! Type 'exit' to quit.\n");

    while (1) {
        // Build current path using directory stack
        Directory* pathStack[100];
        int count = 0;
        Directory* temp = current;
        while (temp != NULL && temp != root) {
            pathStack[count++] = temp;
            temp = temp->parent;
        }

        printf("root");
        for (int i = count - 1; i >= 0; i--) {
            printf("/%s", pathStack[i]->name);
        }
        printf(" >> ");

        fgets(input, sizeof(input), stdin);
        executeCommand(input);
    }

    return 0;
}
