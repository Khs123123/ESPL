#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 10000

typedef struct virus {
    unsigned short SigSize;
    unsigned char* VirusName;
    unsigned char* Sig;
} virus;

typedef struct link link;

struct link {
    link* nextVirus;
    virus* vir;
};

// Function declarations
virus* readVirus(FILE* file);
void printVirus(virus* virus, FILE* output);
void list_print(link* virus_list, FILE* output);
link* list_append(link* virus_list, virus* data);
void list_free(link* virus_list);
void detect_virus(char* buffer, unsigned int size, link* virus_list);
void detect_and_patch(char* buffer, unsigned int size, link* virus_list, const char* fileName);
void neutralize_virus(char* fileName, int signatureOffset);

// Global variables
link* virusList = NULL;
char suspectedFile[256] = {0};
int isBigEndian = 0;

void menu() {
    while (1) {
        printf("Menu:\n");
        printf("1) Load signatures\n");
        printf("2) Print signatures\n");
        printf("3) Detect viruses\n");
        printf("4) Fix file\n");
        printf("5) Quit\n");
        printf("Enter option: ");
        char input[10];
        if (!fgets(input, sizeof(input), stdin)) break;

        int choice = 0;
        sscanf(input, "%d", &choice);
        switch (choice) {
            case 1: {
                printf("Enter signature file name: ");
                char sigFile[256];
                fgets(sigFile, sizeof(sigFile), stdin);
                sigFile[strcspn(sigFile, "\n")] = '\0';

                FILE* file = fopen(sigFile, "rb");
                if (!file) {
                    perror("Failed to open signature file");
                    break;
                }

                char magic[4];
                if (fread(magic, 1, 4, file) != 4) {
                    fprintf(stderr, "Failed to read magic number.\n");
                    fclose(file);
                    break;
                }

                if (strncmp(magic, "VIRL", 4) == 0) {
                    isBigEndian = 0;
                } else if (strncmp(magic, "VIRB", 4) == 0) {
                    isBigEndian = 1;
                } else {
                    fprintf(stderr, "Invalid magic number.\n");
                    fclose(file);
                    break;
                }

                while (!feof(file)) {
                    virus* v = readVirus(file);
                    if (!v) break;
                    virusList = list_append(virusList, v);
                }

                fclose(file);
                break;
            }
            case 2:
                list_print(virusList, stdout);
                break;
            case 3: {
                FILE* file = fopen(suspectedFile, "rb");
                if (!file) {
                    fprintf(stderr, "Error: cannot open %s\n", suspectedFile);
                    break;
                }
                char buffer[BUFFERSIZE];
                size_t readBytes = fread(buffer, 1, BUFFERSIZE, file);
                fclose(file);
                detect_virus(buffer, readBytes, virusList);
                break;
            }
            case 4: {
                FILE* file = fopen(suspectedFile, "rb");
                if (!file) {
                    fprintf(stderr, "Error: cannot open %s\n", suspectedFile);
                    break;
                }
                char buffer[BUFFERSIZE];
                size_t readBytes = fread(buffer, 1, BUFFERSIZE, file);
                fclose(file);

                detect_and_patch(buffer, readBytes, virusList, suspectedFile);
                break;
            }
            case 5:
                list_free(virusList);
                return;
            default:
                printf("Invalid option.\n");
                break;
        }
    }
}

// Helpers

virus* readVirus(FILE* file) {
    virus* v = malloc(sizeof(virus));
    if (!v) return NULL;

    if (fread(&v->SigSize, 1, 2, file) != 2) {
        free(v);
        return NULL;
    }

    if (isBigEndian)
        v->SigSize = (v->SigSize >> 8) | (v->SigSize << 8);

    v->VirusName = malloc(16);
    if (fread(v->VirusName, 1, 16, file) != 16) {
        free(v->VirusName);
        free(v);
        return NULL;
    }

    v->Sig = malloc(v->SigSize);
    if (fread(v->Sig, 1, v->SigSize, file) != v->SigSize) {
        free(v->Sig);
        free(v->VirusName);
        free(v);
        return NULL;
    }

    return v;
}

void printVirus(virus* v, FILE* output) {
    fprintf(output, "Virus name: %s\n", v->VirusName);
    fprintf(output, "Virus size: %u\n", v->SigSize);
    fprintf(output, "Signature:\n");
    for (int i = 0; i < v->SigSize; i++)
        fprintf(output, "%02X ", v->Sig[i]);
    fprintf(output, "\n\n");
}

link* list_append(link* virus_list, virus* data) {
    link* new_node = malloc(sizeof(link));
    new_node->vir = data;
    new_node->nextVirus = virus_list;
    return new_node;
}

void list_print(link* virus_list, FILE* output) {
    while (virus_list) {
        printVirus(virus_list->vir, output);
        virus_list = virus_list->nextVirus;
    }
}

void list_free(link* virus_list) {
    while (virus_list) {
        link* next = virus_list->nextVirus;
        free(virus_list->vir->Sig);
        free(virus_list->vir->VirusName);
        free(virus_list->vir);
        free(virus_list);
        virus_list = next;
    }
}

void detect_virus(char* buffer, unsigned int size, link* virus_list) {
    int found = 0;
    for (int i = 0; i < size; i++) {
        link* curr = virus_list;
        while (curr) {
            virus* v = curr->vir;
            if (i + v->SigSize <= size && memcmp(buffer + i, v->Sig, v->SigSize) == 0) {
                printf("Virus found!\nLocation: %d\nName: %s\nSize: %u\n\n",
                       i, v->VirusName, v->SigSize);
                found = 1;
            }
            curr = curr->nextVirus;
        }
    }
    if (!found) {
        printf("No viruses found in the buffer.\n");
    }
}

void detect_and_patch(char* buffer, unsigned int size, link* virus_list, const char* fileName) {
    int found = 0;
    for (int i = 0; i < size; i++) {
        link* curr = virus_list;
        while (curr) {
            virus* v = curr->vir;
            if (i + v->SigSize <= size && memcmp(buffer + i, v->Sig, v->SigSize) == 0) {
                printf("Neutralizing virus: %s at offset: %d\n", v->VirusName, i);
                neutralize_virus((char*)fileName, i);
                found = 1;
            }
            curr = curr->nextVirus;
        }
    }
    if (!found) {
        printf("No viruses found for patching!\n");
    }
}

void neutralize_virus(char* fileName, int signatureOffset) {
    FILE* f = fopen(fileName, "r+");
    if (!f) {
        perror("Error opening file to neutralize");
        return;
    }
    printf("Writing 0xC3 at offset: %d\n", signatureOffset);

    if (fseek(f, signatureOffset, SEEK_SET) != 0) {
        perror("fseek failed");
        fclose(f);
        return;
    }

    char ret = 0xC3;
    size_t written = fwrite(&ret, 1, 1, f);
    fflush(f);
    fclose(f);

    if (written != 1) {
        fprintf(stderr, "Failed to write byte\n");
    } else {
        printf("Patched successfully!\n");
    }
}

// Main
int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <suspected file>\n", argv[0]);
        return 1;
    }
    strncpy(suspectedFile, argv[1], sizeof(suspectedFile) - 1);
    menu();
    return 0;
}