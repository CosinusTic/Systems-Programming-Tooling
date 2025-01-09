#ifndef BREAK_H
#define BREAK_H

#include <sys/types.h>

struct Breakpoint
{
    unsigned long addr;
    unsigned char original_byte;
    char symbol_name[64];
};

extern struct Breakpoint breakpoints[];
extern int breakpoint_count;

void set_breakpoint(pid_t child_pid, unsigned long addr, const char *sym);
void list_breakpoints(void);
void delete_breakpoint(pid_t child_pid, int index);
int find_breakpoint_index(unsigned long addr);

#endif /* !BREAK_H */
