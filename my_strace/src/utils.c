#include "include/utils.h"

#include <asm/unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>
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

char *handle_open_flags(int flag)
{
    switch (flag)
    {
    case O_RDONLY:
        return "O_RDONLY";
    case O_WRONLY:
        return "O_WRONLY";
    case O_CREAT:
        return "O_CREAT";
    case O_APPEND:
        return "O_APPEND";
    case O_RDWR:
        return "O_RDWR";
    default:
        return "UNKNOWN FLAG";
    }
}
