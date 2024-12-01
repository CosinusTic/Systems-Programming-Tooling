#include <stdio.h>

#include "../include/handler.h"
#include "../include/mapper.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;

    struct file *f = file_map(argv[1]);

    if (!f)
    {
        fprintf(stderr, "The file is of the wrong format\n");
        return 1;
    }

    puts("File is of ELF format");
    file_unmap(&f);
    return 0;
}
