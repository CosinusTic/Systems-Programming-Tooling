#include "include/utils.h"

#define _GNU_SOURCE
#include <asm/unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

void read_strmem(pid_t child, unsigned long addr, char *buffer, size_t max_len)
{
    long data;
    size_t i = 0;

    while (i < max_len)
    {
        data = ptrace(PTRACE_PEEKDATA, child, addr + i, NULL);

        if (data == -1)
            break;
        memcpy(buffer + i, &data, sizeof(data));

        // Check for the null terminator
        if (memchr(&data, '\0', sizeof(data)) != NULL)
        {
            break;
        }

        i += sizeof(data);
    }
}

char *map_access_mode(int flags)
{
    static char buffer[256];
    buffer[0] = '\0';

    switch (flags & O_ACCMODE)
    {
    case O_RDONLY:
        strcat(buffer, "O_RDONLY");
        break;
    case O_WRONLY:
        strcat(buffer, "O_WRONLY");
        break;
    case O_RDWR:
        strcat(buffer, "O_RDWR");
        break;
    default:
        strcat(buffer, "INVALID_ACCESS_MODE");
        break;
    }

    if (flags & O_CREAT)
        strcat(buffer, " | O_CREAT");
    if (flags & O_EXCL)
        strcat(buffer, " | O_EXCL");
    if (flags & O_NOCTTY)
        strcat(buffer, " | O_NOCTTY");
    if (flags & O_TRUNC)
        strcat(buffer, " | O_TRUNC");
    if (flags & O_APPEND)
        strcat(buffer, " | O_APPEND");
    if (flags & O_NONBLOCK)
        strcat(buffer, " | O_NONBLOCK");
    if (flags & O_ASYNC)
        strcat(buffer, " | O_ASYNC");
    if (flags & O_SYNC)
        strcat(buffer, " | O_SYNC");
    if (flags & FD_CLOEXEC)
        strcat(buffer, " | FD_CLOEXEC");

    return buffer;
}

char *map_dirfd(int flag)
{
    // convert numeric value to string for consitent format
    char *buf = malloc(sizeof(char) * 10);
    snprintf(buf, 10, "%d", flag);

    switch (flag)
    {
    case -100:
        return "AT_FCWD";
    default:
        return buf;
    }
}
