#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

void read_strmem(pid_t child, unsigned long addr, char *buffer, size_t max_len);
char *handle_open_flags(int flag);

#endif /* !UTILS_H */
