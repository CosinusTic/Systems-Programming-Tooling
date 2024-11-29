#include "handler.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

/*
struct file
{
    int fd;
    size_t size;
    char *name;
    void *content;
};
*/

struct file *file_map(const char *filename)
{
    struct stat s = { 0 };
    struct file *new = malloc(sizeof(struct file));
    ssize_t bytes_read = 0;
    char magic[4];
    if (new == NULL)
    {
        perror("Cannot malloc a struct file");
        goto error_malloc;
    }

    if ((new->name = malloc(strlen(filename) + 1)) == NULL)
    {
        perror("Cannot malloc a string of strlen(filename)");
        goto error_name;
    }
    new->name = strcpy(new->name, filename);

    if ((new->fd = open(new->name, O_RDONLY)) == -1)
    {
        perror("Cannot open(filename) in read-only");
        goto error_open;
    }

    if ((stat(new->name, &s)) == -1)
    {
        perror("Cannot stat(filename)");
        goto error_stat;
    }
    new->size = s.st_size;

    if ((new->content =
             mmap(NULL, new->size, PROT_READ, MAP_PRIVATE, new->fd, 0))
        == MAP_FAILED)
    {
        perror("Cannot mmap for the file size");
        goto error_mmap;
    }

    if ((bytes_read = read(new->fd, magic, 4)) == -1)
    {
        perror("Failed to read the magic bytes");
        goto error_read;
    }

    new->is_elf = magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L'
            && magic[3] == 'F'
        ? 1
        : 0;

    return new;

error_read:
error_mmap:
error_stat:
    close(new->fd);
error_open:
    free(new->name);
error_name:
    free(new);
error_malloc:
    return NULL;
}

void file_unmap(struct file **f)
{
    free((*f)->name);
    close((*f)->fd);
    munmap((*f)->content, (*f)->size);
    free(*f);
    *f = NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;

    struct file *f = file_map(argv[1]);

    if (!f)
        return 1;

    char *message;

    message = f->is_elf == 1 ? "File is an ELF" : "File is not an ELF";

    printf("Outcome: %s\n", message);

    return 0;
}
