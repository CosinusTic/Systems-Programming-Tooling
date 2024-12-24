#include "include/handler.h"

#include <fcntl.h>

static void read_strmem(pid_t child, unsigned long addr, char *buffer,
                        size_t max_len)
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

static char *handle_open_flags(int flag)
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

void handle_open(struct user_regs_struct *regs, pid_t pid)
{
    char path[256] = { 0 };
    char *flag = handle_open_flags(regs->rsi);
    read_strmem(pid, regs->rdi, path, sizeof(path));
    /*
    printf("open(pathname=\"%s\", flags=0x%llx, mode=0%llo)\n", path, regs->rsi,
           regs->rdx);
    */
    printf("open(pathname=%s, flags=%s, mode=0%llo)\n", path, flag, regs->rdx);
}

void handle_close(struct user_regs_struct *regs)
{
    printf("close(fd=%llu)", regs->rdi);
}

void handle_write(struct user_regs_struct *regs)
{
    int fd = regs->rdi;
    unsigned long long buf = regs->rsi;
    size_t cnt = regs->rdx;

    printf("write(fd=%d, buf=0x%llx, size=%lu)", fd, buf, cnt);
}
