#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

void read_strmem(pid_t child, unsigned long addr, char *buffer, size_t max_len);
char *map_access_mode(int flags);
char *map_dirfd(int flag);

#endif /* !UTILS_H */
