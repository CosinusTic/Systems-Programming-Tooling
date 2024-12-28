#include "include/handler.h"

#include "include/utils.h"

void handle_open(struct user_regs_struct *regs, pid_t pid)
{
    char path[256] = { 0 };
    char *flag = handle_open_flags(regs->rsi);
    read_strmem(pid, regs->rdi, path, sizeof(path));

    printf("open(pathname=%s, flags=%s, mode=0%llo)\n", path, flag, regs->rdx);
}

void handle_openat(struct user_regs_struct *regs, pid_t pid)
{
    int dirfd = regs->rdi;
    char pathbuf[256] = { 0 };
    read_strmem(pid, regs->rsi, pathbuf, sizeof(pathbuf));
    int flags = regs->rdx;
    mode_t mode = regs->r12;

    printf("openat(dirfd=%d, pathname=%s, flags=%d, mode=%d)", dirfd, pathbuf,
           flags, mode);
}

void handle_close(struct user_regs_struct *regs)
{
    printf("close(fd=%llu)", regs->rdi);
}

void handle_read(struct user_regs_struct *regs)
{
    int fd = regs->rdi;
    unsigned long long buf = regs->rsi;
    size_t cnt = regs->rdx;

    printf("write(fd=%d, buf=0x%llx, cnt=%lu)", fd, buf, cnt);
}

void handle_pread(struct user_regs_struct *regs)
{
    int fd = regs->rdi;
    unsigned long long buf = regs->rsi;
    size_t nbytes = regs->rdx;
    off_t offset = regs->r10;

    printf("pread(fd=%d, buf=0x%llx, nbytes=%lu, offset=%lu)", fd, buf, nbytes,
           offset);
}

void handle_write(struct user_regs_struct *regs)
{
    int fd = regs->rdi;
    unsigned long long buf = regs->rsi;
    size_t cnt = regs->rdx;

    printf("write(fd=%d, buf=0x%llx, size=%lu)", fd, buf, cnt);
}

void handle_mmap(struct user_regs_struct *regs)
{
    unsigned long long addr = regs->rdi;
    size_t length = regs->rsi;
    int prot = regs->rdx;
    int flags = regs->r10;
    int fd = regs->r8;
    off_t offset = regs->r9;

    printf(
        "mmap(addr=x0%llx, length=%lu, prot=%d, flags=%d, fd=%d, offset=%lu)",
        addr, length, prot, flags, fd, offset);
}

void handle_munmap(struct user_regs_struct *regs)
{
    unsigned long long addr = regs->rdi;
    size_t length = regs->rsi;

    printf("munmap(addr=0x%llx, length=%lu)", addr, length);
}
