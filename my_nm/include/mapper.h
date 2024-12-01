#ifndef MAPPER_H
#define MAPPER_H

#include <stddef.h>

struct file
{
    int fd;
    size_t size;
    char *name;
    void *content;
};

struct file *file_map(const char *filename);
void file_unmap(struct file **f);

#endif /* !HANDLER_H */
