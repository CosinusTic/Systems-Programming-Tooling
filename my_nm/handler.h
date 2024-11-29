#ifndef HANDLER_H
#define HANDLER_H

#include <stddef.h>

struct file
{
    int fd;
    size_t size;
    int is_elf;
    char *name;
    void *content;
};

struct file *file_mapp(const char *filename);
void file_unmap(struct file **f);
int is_elf(const struct file *f);

#endif /* !HANDLER_H */
