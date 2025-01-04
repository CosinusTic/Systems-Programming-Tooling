#include "include/mapper.h"

#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "include/symbols.h"

static int is_elf(const struct file *f)
{
    Elf64_Ehdr *fheaders = (Elf64_Ehdr *)f->content;
    return (fheaders->e_ident[EI_MAG0] == 0x7f
            && fheaders->e_ident[EI_MAG1] == 'E'
            && fheaders->e_ident[EI_MAG2] == 'L'
            && fheaders->e_ident[EI_MAG3] == 'F')
        ? 1
        : 0;
}

struct file *file_map(const char *filename)
{
    struct stat s = { 0 };
    struct file *new = malloc(sizeof(struct file));

    if (new == NULL)
    {
        perror("Cannot malloc a struct file");
        goto error_malloc;
    }

    if ((new->name = malloc(strlen(filename) + 1)) == NULL)
    {
        perror("Cannot malloc a string of strlen(filename)");
        goto error_name;
    }
    new->name = strcpy(new->name, filename);

    if ((new->fd = open(new->name, O_RDONLY)) == -1)
    {
        perror("Cannot open(filename) in read-only");
        goto error_open;
    }

    if ((stat(new->name, &s)) == -1)
    {
        perror("Cannot stat(filename)");
        goto error_stat;
    }
    new->size = s.st_size;

    if ((new->content =
             mmap(NULL, new->size, PROT_READ, MAP_PRIVATE, new->fd, 0))
        == MAP_FAILED)
    {
        perror("Cannot mmap for the file size");
        goto error_mmap;
    }

    if (!is_elf(new))
        goto error_format;

    return new;

error_format:
error_mmap:
error_stat:
    close(new->fd);
error_open:
    free(new->name);
error_name:
    free(new);
error_malloc:
    return NULL;
}

void file_unmap(struct file **f)
{
    free((*f)->name);
    close((*f)->fd);
    munmap((*f)->content, (*f)->size);
    free(*f);
    *f = NULL;
}

char *map_symvis(unsigned char vis)
{
    switch (vis)
    {
    case STV_DEFAULT:
        return "STV_DEFAULT";
    case STV_INTERNAL:
        return "STV_INTERNAL";
    case STV_HIDDEN:
        return "STV_HIDDEN";
    case STV_PROTECTED:
        return "STV_PROTECTED";
    default:
        return "UNKNOWN";
    }
}

char *map_symtype(unsigned char type) // Map a
{
    switch (type)
    {
    case STT_NOTYPE:
        return "STT_NOTYPE";
    case STT_OBJECT:
        return "STT_OBJECT";
    case STT_FUNC:
        return "STT_FUNC";
    case STT_SECTION:
        return "STT_SECTION";
    case STT_FILE:
        return "STT_FILE";
    default:
        return "UNKNOWN";
    }
}

char *map_symbinding(unsigned char bind)
{
    switch (bind)
    {
    case STB_LOCAL:
        return "STB_LOCAL";
    case STB_GLOBAL:
        return "STB_GLOBAL";
    case STB_WEAK:
        return "STB_WEAK";
    default:
        return "UNKNOWN";
    }
}

struct sym_llist *parse_elf_symbols(const struct file *f)
{
    char *fptr = (char *)f->content;
    Elf64_Ehdr *elf_hdr = (Elf64_Ehdr *)fptr;
    Elf64_Shdr *section_headers =
        (Elf64_Shdr *)(fptr + elf_hdr->e_shoff); // meta data of sections
    const char *shstrtab = (const char *)(fptr
                                          + section_headers[elf_hdr->e_shstrndx]
                                                .sh_offset); // Section names

    Elf64_Shdr *symtab = NULL;
    Elf64_Shdr *strtab = NULL;

    for (int i = 0; i < elf_hdr->e_shnum; i++)
    {
        const char *section_name = shstrtab + section_headers[i].sh_name;
        if (strcmp(section_name, ".symtab") == 0)
            symtab = &section_headers[i];
        else if (strcmp(section_name, ".strtab") == 0)
            strtab = &section_headers[i];
    }

    if (!symtab || !strtab)
        return NULL;

    Elf64_Sym *symbols = (Elf64_Sym *)(fptr + symtab->sh_offset);
    const char *string_table = fptr + strtab->sh_offset;
    int sym_amt = symtab->sh_size / symtab->sh_entsize;
    struct sym_llist *symbol_llist = NULL;

    for (int i = 0; i < sym_amt; i++)
    {
        Elf64_Sym *sym = &(symbols[i]);
        char *sym_name = (char *)&string_table[sym->st_name];

        if (ELF64_ST_TYPE(sym->st_info) == STT_FILE)
            continue;

        char *section_name = "UND";
        if (sym->st_shndx == SHN_UNDEF)
            section_name = "UND";
        else if (sym->st_shndx < SHN_LORESERVE) // Valid section index
            section_name =
                (char *)shstrtab + section_headers[sym->st_shndx].sh_name;

        struct sym_data *symbol_data = malloc(sizeof(struct sym_data));
        if (!symbol_data)
        {
            free_list(symbol_llist);
            return NULL;
        }
        symbol_data->value = sym->st_value;
        symbol_data->size = sym->st_size;
        symbol_data->type = map_symtype(ELF64_ST_TYPE(sym->st_info));
        symbol_data->binding = map_symbinding(ELF64_ST_BIND(sym->st_info));
        symbol_data->visibility =
            map_symvis(ELF64_ST_VISIBILITY(sym->st_other));
        symbol_data->section_name = section_name;
        symbol_data->symbol_name = sym_name;

        struct sym_llist *node = create_node(symbol_data);
        if (!node)
        {
            free_list(symbol_llist);
            return NULL;
        }

        insert_node(&symbol_llist, node);
    }

    return symbol_llist;
}
