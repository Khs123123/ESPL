#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZED_ARRAY 5

char *map(char *array, int array_length, char (*f)(char))
{
    char *mapped_array = malloc(array_length * sizeof(char));

    for (int i = 0; i < array_length; i++)
    {
        mapped_array[i] = f(array[i]);
    }
    return mapped_array;
}

char my_get(char c)
{
    return fgetc(stdin);
}

char cprt(char c)
{
    if (c >= 0x20 && c <= 0x7E)
    {

           printf("%c\n", c);

    }
    else
    {

           printf(".\n");

    }


    return c;
}

char encrypt(char c)
{
    return (c >= 0x21 && c <= 0x7F) ? c - 0x01 : c;
}

char decrypt(char c)
{
    return (c >= 0x1F && c <= 0x7E) ? c + 0x01 : c;
}


char oprt(char c)
{
    if(c >= 0x20 && c <= 0x7E){
        printf("%o\n", c);

    }
    else
    {
        printf(".\n");
    }
    
    return c;
}



typedef struct fun_desc
{
    const char *name;
    char (*fun)(char);
} 
fun_desc;

int main()
{
    char carray[SIZED_ARRAY] = {0};
    char input[100];
    char *carraySave = NULL;

    fun_desc menu[] = {
        {"Get string", my_get},
        {"Print decimal", cprt},
        {"Encrypt", encrypt},
        {"Decrypt", decrypt},
        {"Print OCTAL", oprt},
        {NULL, NULL}};

    while (true)
    {
        printf("Select operation from the following menu (ctrl^D for exit):\n");
        for (int i = 0;i<3; i++)
        {
            
        }
        for (int i = 0; menu[i].name != NULL; i++)
        {
            printf("%d) %s\n", i, menu[i].name);
        }

        printf("Option: ");
        if (!fgets(input, sizeof(input), stdin))
        {
            break;
        }

        int option = atoi(input);
        if (option >= 0 && menu[option].name != NULL)
        {
            printf("Within bounds\n");
            carraySave = map(carray, SIZED_ARRAY, menu[option].fun);
            if (carraySave)
            {
                for (int i = 0; i < SIZED_ARRAY; i++)
                {
                    carray[i] = carraySave[i];
                }
                free(carraySave);
            }
            printf("DONE.\n\n");
        }
        else
        {
            printf("Not within bounds\n");
            exit(0);
        }
    }

    return 0;
}
