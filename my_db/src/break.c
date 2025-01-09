#include "include/break.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_BREAKPOINTS 50

struct Breakpoint breakpoints[MAX_BREAKPOINTS];
int breakpoint_count = 0;

// Set breakpoint instruction at the specified address in the child process
void set_breakpoint(pid_t child_pid, unsigned long addr, const char *sym)
{
    if (breakpoint_count >= MAX_BREAKPOINTS)
    {
        fprintf(stderr, "Maximum number of breakpoints reached.\n");
        return;
    }

    // Read a word at addr
    unsigned long data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)addr, NULL);
    if ((long)data == -1 && errno != 0)
    {
        perror("Failed to read memory for breakpoint");
        return;
    }

    // Save the original first byte then put 0xCC in place
    unsigned char original_byte = data & 0xFF;
    unsigned long data_with_int3 = (data & ~0xFF) | 0xCC;

    // Replace address content with int3 instruction
    if (ptrace(PTRACE_POKETEXT, child_pid, (void *)addr, (void *)data_with_int3)
        == -1)
    {
        perror("Failed to set breakpoint");
        return;
    }

    breakpoints[breakpoint_count].addr = addr;
    breakpoints[breakpoint_count].original_byte = original_byte;
    if (sym)
    {
        strncpy(breakpoints[breakpoint_count].symbol_name, sym, 63);
        breakpoints[breakpoint_count].symbol_name[63] = '\0';
    }
    else
        breakpoints[breakpoint_count].symbol_name[0] = '\0';

    breakpoint_count++;
    printf("Breakpoint set at 0x%lx", addr);
    if (sym && sym[0] != '\0')
        printf(" (%s)", sym);
    printf("\n");
}

void list_breakpoints(void)
{
    printf("Breakpoints:\n");
    for (int i = 0; i < breakpoint_count; i++)
    {
        printf("%d: 0x%lx", i + 1, breakpoints[i].addr);
        if (breakpoints[i].symbol_name[0] != '\0')
            printf(" (%s)", breakpoints[i].symbol_name);
        printf("\n");
    }
}

void delete_breakpoint(pid_t child_pid, int index)
{
    if (index < 0 || index >= breakpoint_count)
    {
        fprintf(stderr, "Invalid breakpoint index.\n");
        return;
    }

    unsigned long addr = breakpoints[index].addr;
    unsigned char original_byte = breakpoints[index].original_byte;

    // Restore the original first byte
    unsigned long data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)addr, NULL);
    if ((long)data == -1 && errno != 0)
    {
        perror("Failed to read memory for breakpoint removal");
        return;
    }
    unsigned long restored_data = (data & ~0xFF) | original_byte;

    if (ptrace(PTRACE_POKETEXT, child_pid, (void *)addr, (void *)restored_data)
        == -1)
    {
        perror("Failed to restore original byte for breakpoint");
        return;
    }

    // Shift the remaining breakpoints down
    for (int i = index; i < breakpoint_count - 1; i++)
        breakpoints[i] = breakpoints[i + 1];

    breakpoint_count--;

    printf("Breakpoint at 0x%lx deleted.\n", addr);
}

int find_breakpoint_index(unsigned long addr)
{
    for (int i = 0; i < breakpoint_count; i++)
        if (breakpoints[i].addr == addr)
            return i;

    return -1;
}
