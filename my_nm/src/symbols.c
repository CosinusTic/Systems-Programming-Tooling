#include "include/symbols.h"

#include <stdio.h>
#include <stdlib.h>

struct sym_llist *create_node(struct sym_data *data)
{
    struct sym_llist *node = malloc(sizeof(struct sym_llist));

    if (!node)
        return NULL;

    node->data = data;
    node->next = NULL;

    return node;
}

void insert_node(struct sym_llist **head, struct sym_llist *new_node)
{
    if (!*head)
        *head = new_node;
    else
    {
        struct sym_llist *current = *head;
        while (current->next)
            current = current->next;

        current->next = new_node;
    }
}

void free_list(struct sym_llist *head)
{
    if (!head)
        return;

    struct sym_llist *current = head;

    while (current)
    {
        free(current->data->type);
        free(current->data->binding);
        free(current->data->visibility);
        free(current->data->section_name);
        free(current->data->symbol_name);
        free(current->data);
        current = current->next;
    }

    free(head);
}

static void print_symbol(struct sym_data *data)
{
    /*
    struct sym_data
    {
        u_int64_t value;
        u_int64_t size;
        char *type;
        char *binding;
        char *visibility;
        char *section_name;
        char *symbol_name;
    };*/
    printf("%016lx\t%lu\t%-12s\t%-12s\t%-12s\t%s\t%s\n", data->value,
           data->size, data->type, data->binding, data->visibility,
           data->section_name, data->symbol_name);
}

void sym_llist_traversal(struct sym_llist *head)
{
    if (!head)
        return;

    struct sym_llist *current = head;
    print_symbol(head->data);
    while (current->next)
    {
        print_symbol(current->next->data);
        current = current->next;
    }
}
