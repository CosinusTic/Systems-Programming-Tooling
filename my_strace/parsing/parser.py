txt = open("syscalls.txt")
out = open("syscalls.c", "w")

syscalls = []
for l in txt:
    words = l.split("\t")
    name = words[2]
    syscalls.append(name)

out.write("/*\n-- This is a generated file from a custom python parser\n")
out.write("-- Author: Nathan Delmarche\n\n")
out.write("This file intends to include all headers as a list to be able to map syscalls with their name\n\n*/\n")
out.write('\n#include \"include/syscalls.h\"\n\n')
out.write(f"const char *syscalls[{len(syscalls)}] = ")
out.write("{\n")
for syscall in syscalls:
    out.write(f'\t\"{syscall}\",\n')
out.write("};\n\n")

txt.close()
out.close()
