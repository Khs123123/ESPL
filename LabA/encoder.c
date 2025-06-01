#include <stdio.h>
#include <string.h>
#include <ctype.h>
FILE *infile = NULL;
FILE *outfile = NULL;
int debug_mode = 1;
char *encoding_key = "0"; 
int encoding_type = 0;    

int is_encodable(char c) { return (isdigit(c) || isalpha(c)); }

char encode(char c)
{
    static int key_index = 0; 
    if(c=='\n'){
        key_index = (key_index + 1) % strlen(encoding_key);
        return c ;
    }
    if (!is_encodable(c))
    {
        return c; 
    }
    int key_value = encoding_key[key_index] - '0';
    
    if (isupper(c))
    {
        c = (c - 'A' + encoding_type * key_value + 26) % 26 + 'A';
    }
 
    else if (isdigit(c))
    {
        c = (c - '0' + encoding_type * key_value + 10) % 10 + '0';
    }
    
    
    key_index = (key_index + 1) % strlen(encoding_key);
    return c;
}
int main(int argc, char *argv[])
{
    infile = stdin;
    outfile = stdout;
    for (int i = 1; i < argc; i++)
    {
       
        if (strcmp(argv[i], "-d") == 0)
        {
            debug_mode = 0;
        }
        else if (strcmp(argv[i], "+d") == 0)
        {
            debug_mode = 1;
        }else if (strncmp(argv[i], "+e", 2) == 0)
        {
            encoding_key = argv[i] + 2;
            
            encoding_type = 1;
            
        }
        else if (strncmp(argv[i], "-e", 2) == 0)
        {
            encoding_key = argv[i] + 2;
             
            encoding_type = -1;
            
        }
        else if (strncmp(argv[i], "-i", 2) == 0)
        {
            infile = fopen(argv[i] + 2, "r");
            if (!infile)
            {
                fprintf(stderr, "Error: Cannot open input file %s\n", argv[i] + 2);
                return 1;
            }
        }
        else if (strncmp(argv[i], "-o", 2) == 0)
        {
            outfile = fopen(argv[i] + 2, "w");
            if (!outfile)
            {
                fprintf(stderr, "Error: Cannot open output file %s\n", argv[i] + 2);
                return 1;
            }
        }
     
        int c;
        while ((c = fgetc(infile)) != EOF)
        {
            char encoded_char = encode(c);
            fputc(encoded_char, outfile);
        }
        if (infile != stdin)
            fclose(infile);
        if (outfile != stdout)
            fclose(outfile);
    }

    return 0;
}