#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>
#include <unistd.h>
#include <string.h>

void *map_start = NULL;
extern int startup(int argc, char** argv, int (*func)(int, char**));
int loop_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;

    if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
        fprintf(stderr, "Error: Not a 32-bit ELF file.\n");
        return -1;
    }

    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + ehdr->e_phoff);
    int phnum = ehdr->e_phnum;

    int i = 0;
    while(i < phnum) {
        func(&phdr[i], i);
        i++;
    }

    return 0;
}

void print_phdr(Elf32_Phdr *phdr, int phnum) {
    const char *type;
    switch (phdr->p_type) {
        case PT_NULL:
            type = "NULL";
            break;
        case PT_LOAD:
            type = "LOAD";
            break;
        case PT_DYNAMIC:
            type = "DYNAMIC";
            break;
        case PT_INTERP:
            type = "INTERP";
            break;
        case PT_NOTE:
            type = "NOTE";
            break;
        case PT_SHLIB:
            type = "SHLIB";
            break;
        case PT_PHDR:
            type = "PHDR";
            break;
        case PT_TLS:
            type = "TLS";
            break;
        default:
            type = "UNKNOWN";
            break;
    }

    char flags[4];
    flags[0] = ' ';
    flags[1] = ' ';
    flags[2] = ' ';
    flags[3] = '\0';
    if (phdr->p_flags & PF_R)
        flags[0] = 'R';
    if (phdr->p_flags & PF_W)
        flags[1] = 'W';
    if (phdr->p_flags & PF_X)
        flags[2] = 'X';

    int prot_flags = 0;
    if (phdr->p_flags & PF_R)
        prot_flags |= PROT_READ;
    if (phdr->p_flags & PF_W)
        prot_flags |= PROT_WRITE;
    if (phdr->p_flags & PF_X)
        prot_flags |= PROT_EXEC;

    int map_flags = MAP_PRIVATE | MAP_FIXED;
    if (phdr->p_flags & PF_W)
        map_flags |= MAP_SHARED;
    printf("%-7s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %s %x  %d  %d\n",
           type, phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz,
           phdr->p_memsz, flags, phdr->p_align, prot_flags, map_flags);
}

void load_phdr(Elf32_Phdr *phdr, int fd) {
    void* vaddr = (void* )(phdr->p_vaddr&0xfffff000);
    off_t offset = phdr->p_offset&0xfffff000;
    size_t padding = phdr->p_vaddr & 0xfff;
    //void *vaddr = (void *)phdr->p_vaddr;
    size_t filesz = phdr->p_filesz;
    size_t memsz = phdr->p_memsz;
    int prot = 0;
    if (phdr->p_flags & PF_R)
        prot |= PROT_READ;
    if (phdr->p_flags & PF_W)
        prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X)
        prot |= PROT_EXEC;

    void *map = mmap(vaddr, memsz + padding, prot, MAP_PRIVATE | MAP_FIXED, fd, offset);
    if (map == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Error getting file size");
        close(fd);
        return 1;
    }

    map_start = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return 1;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + ehdr->e_phoff);
    printf("Type    Offset   VirtAddr   PhysAddr   FileSiz MemSiz Flg Align\n");

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            load_phdr(&phdr[i], fd);
            print_phdr(&phdr[i], fd);
        }
    }
    //loop_phdr(map_start, print_phdr, 0);
    //munmap(map_start, st.st_size);
    close(fd);

    startup(argc - 1, argv + 1, (void *)(ehdr->e_entry));
    munmap(map_start, st.st_size);
}