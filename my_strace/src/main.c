#include <asm/unistd.h>
#include <asm/unistd_64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "include/handler.h"
#include "include/syscalls.h"

int handle_syscalls(struct user_regs_struct *regs, pid_t pid)
{
    switch (regs->orig_rax)
    {
    case __NR_open:
        handle_open(regs, pid);
        return 1;
    case __NR_close:
        handle_close(regs);
        return 1;
    case __NR_write:
        handle_write(regs);
        return 1;
    case __NR_read:
        handle_read(regs);
        return 1;
    case __NR_pread64:
        handle_pread(regs);
        return 1;
    case __NR_mmap:
        handle_mmap(regs);
        return 1;
    case __NR_munmap:
        handle_munmap(regs);
        return 1;
    case __NR_openat:
        handle_openat(regs, pid);
        return 1;
    default:
        return 0;
    }
}

int main(int argc, char *argv[], char *envp[])
{
    if (argc != 2)
        return 1;

    int status = 0;
    long pt = 0;
    struct user_regs_struct regs = { 0 };
    int pid = fork();

    switch (pid)
    {
    case -1:
        return 2;

    case 0:
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execve(argv[1], argv + 1, envp);

    default:
        break;
    }

    while (1)
    {
        pid_t w = waitpid(pid, &status, 0);
        if (w == -1)
            perror("Error on waitpid");

        if (WIFEXITED(status)) // Exit from child program
        {
            int code = WEXITSTATUS(status);
            printf("Program exited with code %d\n", code);
            break;
        }
        pt = ptrace(PTRACE_PEEKUSER, pid, 8 * ORIG_RAX, NULL);
        ptrace(PTRACE_GETREGS, pid, NULL, &regs);

        if (handle_syscalls(&regs, pid) != 1)
            printf("%s() = %lld\n", syscalls[pt], regs.rax);
        else
            printf(" = %lld\n", regs.rax);

        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    }

    return 0;
}
