// make_configure.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char* argv[])
{
    FILE* temp_file = tmpfile();
    if(temp_file == NULL)
    {
        printf("Could not open temporary file for writing.");
        perror("  Error message = ");
    }

    const char* const makefilename = "Makefile";
    FILE* makefile = fopen(makefilename, "r");
    if(makefile == NULL)
    {
        printf("Could not open %s for reading.", makefilename);
        perror("  Error message = ");
        exit(1);
    }

    char buffer[5000];
    while(fgets (buffer, sizeof(buffer), makefile))
    {
        if(strstr(buffer, "OPSYS="))
        {
            #ifdef WIN32
            fputs("OPSYS=windows\n", temp_file);
            #else
            fputs("OPSYS=linux\n", temp_file);
            #endif
        }
        else
        {
            fputs(buffer, temp_file);
        }
    }
    fclose(makefile);
    rewind(temp_file);

    makefile = fopen(makefilename, "w");
    if(makefile == NULL)
    {
        printf("Could not open %s for writing.", makefilename);
        perror("  Error message = ");
        exit(1);
    }

    while(fgets (buffer, sizeof(buffer), temp_file))
    {
        fputs(buffer, makefile);
    }
    fclose(temp_file);
    fclose(makefile);
    
    return 0;
}
