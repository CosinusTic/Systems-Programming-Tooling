#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "include/syscalls.h"

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
            int stat = WEXITSTATUS(status);
            printf("Program terminated with exit code %d\n", stat);
            break;
        }

        pt = ptrace(PTRACE_PEEKUSER, pid, 8 * ORIG_RAX, NULL);
        ptrace(PTRACE_GETREGS, pid, NULL, &regs);
        printf("syscall: %ld: %s\nrdi: %lld\nrsi: %lld\nrdx: %lld\n\n", pt,
               syscallnames[pt], regs.rdi, regs.rsi, regs.rdx);

        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    }

    return 0;
}
