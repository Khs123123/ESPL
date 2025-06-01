#include <stdio.h>

int count_digits(char *input){
    int digits = 0;
    unsigned int i = 0;

    while (input[i] != '\0')
    {
        if (input[i] >= '0' && input[i] <= '9'){
            digits++;
        }
        i++;
    }
    return digits;
}


int main(int argc, char *argv[])
{
    if (argc < 2){
        printf ("there is no thing to check");
        return 1;
    }
    printf("String contains %d digits \n", count_digits(argv[1]));
    return 0;
}
