#include "../include/handler.h"

#include <stdio.h>

#include "../include/mapper.h"

void handle_headers(const struct file *f)
{
    if (!f)
        return;
    printf("Hello");
    return;
}
