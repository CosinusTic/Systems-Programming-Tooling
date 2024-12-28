#ifndef HANDLER_H
#define HANDLER_H

#include <asm/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

void handle_open(struct user_regs_struct *regs, pid_t pid);
void handle_close(struct user_regs_struct *regs);
void handle_write(struct user_regs_struct *regs);
void handle_read(struct user_regs_struct *regs);
void handle_pread(struct user_regs_struct *regs);
void handle_mmap(struct user_regs_struct *regs);
void handle_munmap(struct user_regs_struct *regs);
void handle_openat(struct user_regs_struct *regs, pid_t pid);

#endif /* !HANDLER_H */
