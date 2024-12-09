#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <sys/types.h>

struct sym_data
{
    u_int64_t value;
    u_int64_t size;
    char *type;
    char *binding;
    char *visibility;
    char *section_name;
    char *symbol_name;
};

struct sym_llist
{
    struct sym_data *data;
    struct sym_llist *next;
};

struct sym_llist *create_node(struct sym_data *data);
void insert_node(struct sym_llist **head, struct sym_llist *new_node);
void free_list(struct sym_llist *head);
void sym_llist_traversal(struct sym_llist *head);

#endif /* !SYMBOLS_H */
