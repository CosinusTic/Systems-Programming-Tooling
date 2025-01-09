#define _POSIX_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "include/break.h"
#include "include/mapper.h"
#include "include/symbols.h"
#include "include/utils.h"
#define CMD_MAX_LEN 100
#define MAX_BREAKPOINTS 50

// SIGTRAP already called at this point
static void handle_breakpoint_hit(pid_t child_pid)
{
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, child_pid, NULL, &regs) == -1)
    {
        perror("handle_breakpoint_hit: ptrace GETREGS failed");
        return;
    }

    // RIP is one byte past the int3.
    unsigned long rip = regs.rip - 1;

    int bp_index = find_breakpoint_index(rip);
    if (bp_index == -1)
    {
        fprintf(stderr, "Warning: hit unknown breakpoint at 0x%lx\n", rip);
        return;
    }

    // Restore program state
    regs.rip = rip;
    if (ptrace(PTRACE_SETREGS, child_pid, NULL, &regs) == -1)
    {
        perror("handle_breakpoint_hit: ptrace SETREGS failed");
        return;
    }

    // Restore original byte at that address
    unsigned long data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)rip, NULL);
    if ((long)data == -1 && errno != 0)
    {
        perror("handle_breakpoint_hit: PEEKTEXT failed");
        return;
    }
    unsigned char orig_byte = breakpoints[bp_index].original_byte;
    unsigned long restored_data = (data & ~0xFF) | orig_byte;
    if (ptrace(PTRACE_POKETEXT, child_pid, (void *)rip, (void *)restored_data)
        == -1)
    {
        perror("handle_breakpoint_hit: POKETEXT restore failed");
        return;
    }

    // Singlestep -> give control back to child
    if (ptrace(PTRACE_SINGLESTEP, child_pid, NULL, NULL) == -1)
    {
        perror("handle_breakpoint_hit: SINGLESTEP failed");
        return;
    }

    int status = 0;
    waitpid(child_pid, &status, 0);
    if (WIFEXITED(status))
        return;

    data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)rip, NULL);
    if ((long)data == -1 && errno != 0)
    {
        perror("handle_breakpoint_hit: re-insert read failed");
        return;
    }
    unsigned long data_with_int3 = (data & ~0xFF) | 0xCC;
    if (ptrace(PTRACE_POKETEXT, child_pid, (void *)rip, (void *)data_with_int3)
        == -1)
    {
        perror("handle_breakpoint_hit: re-insert POKETEXT failed");
        return;
    }

    printf("\n[Breakpoint %d hit at 0x%lx]\n", bp_index + 1, rip);
}

static void dump_registers(pid_t child_pid)
{
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, child_pid, NULL, &regs) == -1)
    {
        handle_error("Failed to get registers");
    }

    printf("rax: 0x%llx\n", (unsigned long long)regs.rax);
    printf("rbx: 0x%llx\n", (unsigned long long)regs.rbx);
    printf("rcx: 0x%llx\n", (unsigned long long)regs.rcx);
    printf("rdx: 0x%llx\n", (unsigned long long)regs.rdx);
    printf("rsi: 0x%llx\n", (unsigned long long)regs.rsi);
    printf("rdi: 0x%llx\n", (unsigned long long)regs.rdi);
    printf("rbp: 0x%llx\n", (unsigned long long)regs.rbp);
    printf("rsp: 0x%llx\n", (unsigned long long)regs.rsp);
    printf("r8: 0x%llx\n", (unsigned long long)regs.r8);
    printf("r9: 0x%llx\n", (unsigned long long)regs.r9);
    printf("r10: 0x%llx\n", (unsigned long long)regs.r10);
    printf("r11: 0x%llx\n", (unsigned long long)regs.r11);
    printf("r12: 0x%llx\n", (unsigned long long)regs.r12);
    printf("r13: 0x%llx\n", (unsigned long long)regs.r13);
    printf("r14: 0x%llx\n", (unsigned long long)regs.r14);
    printf("r15: 0x%llx\n", (unsigned long long)regs.r15);
    printf("rip: 0x%llx\n", (unsigned long long)regs.rip);
    printf("eflags: 0x%llx\n", (unsigned long long)regs.eflags);
    printf("cs: 0x%llx\n", (unsigned long long)regs.cs);
    printf("ss: 0x%llx\n", (unsigned long long)regs.ss);
    printf("ds: 0x%llx\n", (unsigned long long)regs.ds);
    printf("es: 0x%llx\n", (unsigned long long)regs.es);
    printf("fs: 0x%llx\n", (unsigned long long)regs.fs);
    printf("gs: 0x%llx\n", (unsigned long long)regs.gs);
    printf("orig_rax: 0x%llx\n", (unsigned long long)regs.orig_rax);
}

static void dump_memory(pid_t child_pid, unsigned long addr, int count,
                        const char *format)
{
    for (int i = 0; i < count; i++)
    {
        unsigned long data = ptrace(PTRACE_PEEKTEXT, child_pid,
                                    (void *)(addr + i * sizeof(long)), NULL);
        if ((long)data == -1 && errno != 0)
        {
            perror("Failed to read memory");
            break;
        }

        unsigned long mem_addr = addr + i * sizeof(long);
        if (strcmp(format, "x") == 0)
            printf("0x%lx: 0x%lx\n", mem_addr, data);
        else if (strcmp(format, "d") == 0)
            printf("0x%lx: %ld\n", mem_addr, (long)data);
        else if (strcmp(format, "u") == 0)
            printf("0x%lx: %lu\n", mem_addr, (unsigned long)data);
    }
}

static int wait_and_handle_signal(pid_t child_pid)
{
    int status = 0;
    waitpid(child_pid, &status, 0);

    if (WIFEXITED(status)) // child process exited
    {
        printf("Child process exited with status %d\n", WEXITSTATUS(status));
        return 0;
    }

    if (WIFSTOPPED(status))
    {
        int sig = WSTOPSIG(status);
        if (sig == SIGTRAP)
            handle_breakpoint_hit(child_pid);
        else
            printf("Child stopped by signal %d\n", sig);
    }

    return 1;
}

static void handle_command(const char *command, pid_t child_pid,
                           struct sym_llist *symlist)
{
    if (strcmp(command, "quit") == 0)
    {
        kill(child_pid, SIGKILL);
        waitpid(child_pid, NULL, 0);
        printf("Debugger terminated.\n");
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(command, "kill") == 0)
    {
        kill(child_pid, SIGKILL);
        waitpid(child_pid, NULL, 0);
        printf("Process killed.\n");
    }
    else if (strcmp(command, "registers") == 0)
    {
        dump_registers(child_pid);
    }
    else if (strcmp(command, "continue") == 0)
    {
        if (ptrace(PTRACE_CONT, child_pid, NULL, NULL) == -1)
            perror("Failed to continue process");

        wait_and_handle_signal(child_pid);
    }
    else if (strncmp(command, "x ", 2) == 0 || strncmp(command, "d ", 2) == 0
             || strncmp(command, "u ", 2) == 0)
    {
        char format[2];
        int count = 0;
        char addr_str[64];
        if (sscanf(command, "%1s %d %63s", format, &count, addr_str) == 3)
        {
            unsigned long maybe_addr = lookup_symbol_address(symlist, addr_str);
            if (maybe_addr == 0)
            {
                printf("Could not parse address or symbol: %s\n", addr_str);
                return;
            }
            dump_memory(child_pid, maybe_addr, count, format);
        }
        else
        {
            printf("Usage: x|d|u <count> <addr or symbol>\n");
        }
    }
    else if (strncmp(command, "break ", 6) == 0)
    {
        const char *arg = command + strlen("break ");
        unsigned long addr = lookup_symbol_address(symlist, arg);
        if (addr == 0)
        {
            printf("Could not find symbol \"%s\"\n", arg);
            return;
        }
        set_breakpoint(child_pid, addr, arg);
    }
    else if (strcmp(command, "blist") == 0)
    {
        list_breakpoints();
    }
    else if (strncmp(command, "bdel ", 5) == 0)
    {
        int index = 0;
        if (sscanf(command, "bdel %d", &index) == 1)
        {
            delete_breakpoint(child_pid, index - 1);
        }
        else
        {
            printf("Usage: bdel <breakpoint_number>\n");
        }
    }
    else if (strncmp(command, "next", 4) == 0)
    {
        int steps = 1;
        if (strlen(command) > 4)
            if (sscanf(command, "next %d", &steps) != 1)
                steps = 1;
        for (int i = 0; i < steps; i++)
        {
            if (ptrace(PTRACE_CONT, child_pid, NULL, NULL) == -1)
            {
                perror("Failed to continue process");
                break;
            }
            wait_and_handle_signal(child_pid);
        }
    }
    else
        printf("Unknown command: %s\n", command);
}

static void debugger_loop(pid_t child_pid, struct sym_llist *symlist)
{
    char command[CMD_MAX_LEN];

    while (1)
    {
        printf("my_db> ");
        fflush(stdout);

        if (!fgets(command, CMD_MAX_LEN, stdin))
        {
            printf("Exiting debugger.\n");
            break;
        }
        command[strcspn(command, "\n")] = '\0'; // strip newline

        handle_command(command, child_pid, symlist);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s /path/to/binary\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct file *f = file_map(argv[1]);
    if (!f)
        exit(EXIT_FAILURE);

    struct sym_llist *symlist = parse_elf_symbols(f);
    if (!symlist)
    {
        file_unmap(&f);
        exit(EXIT_FAILURE);
    }

    pid_t child_pid = fork();
    if (child_pid == -1)
    {
        file_unmap(&f);
        handle_error("fork failed");
    }

    if (child_pid == 0) // Child
    {
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1)
        {
            file_unmap(&f);
            handle_error("PTRACE_TRACEME failed");
        }
        execl(argv[1], argv[1], NULL);
        file_unmap(&f);
        handle_error("execl failed");
    }
    else // Parent
    {
        waitpid(child_pid, NULL, 0);

        debugger_loop(child_pid, symlist);

        file_unmap(&f);
        free_list(symlist);
    }

    return 0;
}
