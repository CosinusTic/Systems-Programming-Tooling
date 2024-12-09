#ifndef MAPPER_H
#define MAPPER_H

#include <stddef.h>

#include "symbols.h"

struct file
{
    int fd;
    size_t size;
    char *name;
    void *content;
};

struct file *file_map(const char *filename);
void file_unmap(struct file **f);
struct sym_llist *parse_elf_symbols(const struct file *f);

#endif /* !HANDLER_H */
