#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

// ----------------------
// Global Variables
// ----------------------
char debug_mode = 0;
char file_name[128] = "";
int unit_size = 1;
unsigned char mem_buf[10000];
size_t mem_count = 0;
char display_mode = 0; // 0 = hex, 1 = decimal

// ----------------------
// Menu Declarations
// ----------------------
void toggle_debug_mode();
void set_file_name();
void set_unit_size();
void load_into_memory();
void toggle_display_mode();
void memory_display();
void save_into_file();
void memory_modify();
void quit();

void print_units(FILE* output, char* buffer, int count);

typedef struct {
    char* name;
    void (*func)();
} fun_desc;

// ----------------------
// Menu Definition
// ----------------------
fun_desc menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Set File Name", set_file_name},
    {"Set Unit Size", set_unit_size},
    {"Load Into Memory", load_into_memory},
    {"Toggle Display Mode", toggle_display_mode},
    {"Memory Display", memory_display},
    {"Save Into File", save_into_file},
    {"Memory Modify", memory_modify},
    {"Quit", quit},
    {NULL, NULL}
};

// ----------------------
// Menu Display Function
// ----------------------
void print_menu() {
    if (debug_mode) {
        fprintf(stderr, "Debug: unit_size = %d, file_name = %s, mem_count = %zu\n",
                unit_size, file_name, mem_count);
    }
    printf("Choose action:\n");
    for (int i = 0; menu[i].name != NULL; i++)
        printf("%d - %s\n", i, menu[i].name);
}

// ----------------------
// Menu Option Functions
// ----------------------

void toggle_debug_mode() {
    debug_mode = !debug_mode;
    printf("Debug flag now %s\n", debug_mode ? "on" : "off");
}

void set_file_name() {
    printf("Enter file name: ");
    fgets(file_name, sizeof(file_name), stdin);
    file_name[strcspn(file_name, "\n")] = '\0'; // Remove newline
    if (debug_mode)
        fprintf(stderr, "Debug: file name set to '%s'\n", file_name);
}

void set_unit_size() {
    int size;
    printf("Enter unit size (1, 2, or 4): ");
    scanf("%d", &size);
    while (getchar() != '\n'); // clear buffer
    if (size == 1 || size == 2 || size == 4) {
        unit_size = size;
        if (debug_mode)
            fprintf(stderr, "Debug: set size to %d\n", unit_size);
    } else {
        printf("Invalid unit size\n");
    }
}

void load_into_memory() {
    if (file_name[0] == '\0') {
        printf("Error: file name is empty\n");
        return;
    }

    FILE* f = fopen(file_name, "rb");
    if (!f) {
        perror("Error opening file");
        return;
    }

    char input[256];
    unsigned int location;
    int length;

    printf("Please enter <location> <length>\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &location, &length);

    if (debug_mode)
        fprintf(stderr, "Debug: location = 0x%X, length = %d\n", location, length);

    fseek(f, location, SEEK_SET);
    size_t read_count = fread(mem_buf, unit_size, length, f);
    fclose(f);
    mem_count = read_count;

    printf("Loaded %zu units into memory\n", read_count);
}

void toggle_display_mode() {
    display_mode = !display_mode;
    printf("Display flag now %s, %s representation\n",
           display_mode ? "on" : "off",
           display_mode ? "decimal" : "hexadecimal");
}

// Updated to use print_units for Task 1c
void memory_display() {
    char input[256];
    unsigned int addr;
    int length;

    printf("Enter address and length: ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &addr, &length);

    void* start = (addr == 0) ? mem_buf : (void*)addr;
    print_units(stdout, (char*)start, length);
}

void save_into_file() {
    char input[256];
    unsigned int src_addr, dst_offset;
    int length;

    printf("Please enter <source-address> <target-location> <length>\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %x %d", &src_addr, &dst_offset, &length);

    unsigned char* src = (src_addr == 0) ? mem_buf : (unsigned char*)src_addr;

    FILE* f = fopen(file_name, "r+b");
    if (!f) {
        perror("Error opening file");
        return;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    if (dst_offset > file_size) {
        printf("Error: target offset is beyond file size\n");
        fclose(f);
        return;
    }

    if (debug_mode)
        fprintf(stderr, "Debug: Writing %d units from 0x%x to offset 0x%x\n",
                length, (unsigned int)src, dst_offset);

    fseek(f, dst_offset, SEEK_SET);
    fwrite(src, unit_size, length, f);
    fclose(f);
}

void memory_modify() {
    char input[256];
    int location, val;

    printf("Please enter <location> <val>\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %x", &location, &val);

    if (location < 0 || location + unit_size > sizeof(mem_buf)) {
        printf("Error: location out of bounds\n");
        return;
    }

    memcpy(&mem_buf[location], &val, unit_size);
}

void quit() {
    if (debug_mode)
        printf("quitting\n");
    exit(0);
}

// ----------------------
// Task 1 Helper Function
// ----------------------
// print_units implementation from units.c logic
void print_units(FILE* output, char* buffer, int count) {
    static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit\n", "%#x\n"};
    static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit\n", "%#d\n"};

    for (int i = 0; i < count; i++) {
        int val = *((int*)(buffer + i * unit_size));
        if (display_mode)
            fprintf(output, dec_formats[unit_size - 1], val);
        else
            fprintf(output, hex_formats[unit_size - 1], val);
    }
}

// ----------------------
// Main Loop
// ----------------------
int main() {
    while (1) {
        print_menu();
        printf("Option: ");
        int choice;
        if (scanf("%d", &choice) == 1 && choice >= 0 && choice <= 8) {
            while (getchar() != '\n'); // clear buffer
            menu[choice].func();
        } else {
            printf("Invalid option\n");
            while (getchar() != '\n');
        }
    }
    return 0;
}