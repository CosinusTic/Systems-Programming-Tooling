#include "include/symbols.h"

#include "stdlib.h"

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
