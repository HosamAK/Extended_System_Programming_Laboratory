///////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int debugMode;
int CurrentFileDirectory = -1;

struct fun_desc
{
    char *name;
    void (*fun)();
};

typedef struct
{
    char filename[102];
    char *map_start;               // will point to the start of the memory mapped file
    struct stat fileDirectoryStat; // this is needed to  the size of the file
    Elf32_Ehdr *header;            // this will point to the header structure
} ElfFileInfo;

ElfFileInfo *elfFiles[100];
int numOfElfFiles;

void toggleDebugMode();
void examineELFfile();
void printSectionNames();
void printSymbols();
void checkFilesForMerge();
void mergeELFfiles();
void quit();

/*HELPER FUNCTIONS*/
//for Quit operation:
void closeFileDirectory();
void unmapMemory();
//for examineElfFile operation:
void printRequiredData(ElfFileInfo *elfFile);
//for printSectionNames operation:
void printRequiredDataInDebugMode(int idx, Elf32_Shdr *section, ElfFileInfo *elfFile);
//for checkFilesForMerge operation:
Elf32_Shdr *thereIsOneSymtab(ElfFileInfo *info);
void checkIfSymtabIsUndefined(Elf32_Sym *symtab, int sizeOfSymtab, char *strtab, char *symName);
void checkIfSymtabIsDefined(Elf32_Sym *symtab, int sizeOfSymtab, char *strtab, char *symName);
//for mergeELFfiles operation:
int findNewIdx(Elf32_Shdr *shdr, int size, char *shstrtab, char *name);

int main(int argc, char *argv[])
{
    ElfFileInfo *elfFile = malloc(sizeof(ElfFileInfo));
    debugMode = 0;
    numOfElfFiles = 0;

    struct fun_desc menu[] = {{"Toggle Debug Mode", toggleDebugMode},
                              {"Examine ELF File", examineELFfile},
                              {"Print Section Names", printSectionNames},
                              {"Print Symbols", printSymbols},
                              {"Check Files for Merge", checkFilesForMerge},
                              {"Merge ELF Files", mergeELFfiles},
                              {"Quit", quit},
                              {NULL, NULL}};

    while (true)
    {
        printf("Please choose a function:\n");
        for (int i = 0; i < 7; i++)
        {
            printf("(%d) %s\n", i, menu[i].name);
        }
        printf("Option : ");

        int option;
        if (scanf("%d", &option) == EOF)
        {
            printf("\n\n");
            return 0;
        }
        if (option < 0 || option > 6)
        {
            printf("Not within bounds\n");
            return 0;
        }
        printf("Within bounds\n");

        menu[option].fun();
        printf("\n");
    }
    return 0;
}

void toggleDebugMode()
{
    if (debugMode == 0)
    {
        printf("debug flag now : on\n");
        debugMode = 1;
    }
    else
    {
        printf("debug flag now : off\n");
        debugMode = 0;
    }
}

void examineELFfile()
{
    ElfFileInfo *examinedElfFile = malloc(sizeof(ElfFileInfo));
    char fileName[102];
    printf("enter a new ELF file name: ");
    if (scanf("%s", fileName) == EOF)
    {
        fileName[strlen(fileName) - 1] = 0;
    }

    CurrentFileDirectory = open(fileName, O_RDONLY);
    if (CurrentFileDirectory < 0)
    {
        perror("error in open file directory");
        exit(-1);
    }

    int fst = fstat(CurrentFileDirectory, &examinedElfFile->fileDirectoryStat);
    if (fst != 0)
    {
        close(CurrentFileDirectory);
        CurrentFileDirectory = -1;
        perror("stat failed");
        exit(-1);
    }
    //we should map the examined file
    if ((examinedElfFile->map_start = mmap(0, examinedElfFile->fileDirectoryStat.st_size, PROT_READ, MAP_PRIVATE, CurrentFileDirectory, 0)) == MAP_FAILED)
    {
        //the mmap has failed
        close(CurrentFileDirectory);
        CurrentFileDirectory = -1;
        perror("mmap failed");
        exit(-4);
    }

    examinedElfFile->header = (Elf32_Ehdr *)examinedElfFile->map_start;
    strcpy(examinedElfFile->filename, fileName);
    printRequiredData(examinedElfFile);

    elfFiles[numOfElfFiles] = examinedElfFile;
    numOfElfFiles++;
}

void printSectionNames()
{
    if (numOfElfFiles == 0)
    {
        fprintf(stderr, "You need to examine a file first!\n");
        return;
    }
    for (int i = 0; i < numOfElfFiles; i++)
    {
        ElfFileInfo *elfFile = elfFiles[i];
        if (CurrentFileDirectory == -1)
        {
            printf("\nThere is NO FILE TO DISPLAY because current file directory is -1\n");
            return;
        }
        printf("\nSections for %s ELF file :\n", elfFiles[i]->filename);
        printf("(in this ORDER: index secName secAddr secOff secSize secType)\n\n");

        int idx;
        int sizeOfTheSectionArray = elfFile->header->e_shnum;
        Elf32_Shdr *section;
        section = (Elf32_Shdr *)(elfFile->map_start + elfFile->header->e_shoff);
        char *strTable;
        char typeOfTheFile[16];

        idx = 0;
        while (idx < sizeOfTheSectionArray)
        {
            Elf32_Word sectionName = section[idx].sh_name;
            Elf32_Addr sectionAddress = section[idx].sh_addr;
            Elf32_Off sectionOffset = section[idx].sh_offset;
            Elf32_Word sectionSize = section[idx].sh_size;
            Elf32_Word sectionType = section[idx].sh_type;

            switch (sectionType)
            {
            case SHT_NULL:
                strcpy(typeOfTheFile, "NULL");
                break;
            case SHT_PROGBITS:
                strcpy(typeOfTheFile, "PROGBITS");
                break;
            case SHT_SYMTAB:
                strcpy(typeOfTheFile, "SYMTAB");
                break;
            case SHT_STRTAB:
                strcpy(typeOfTheFile, "STRTAB");
                break;
            case SHT_RELA:
                strcpy(typeOfTheFile, "RELA");
                break;
            case SHT_HASH:
                strcpy(typeOfTheFile, "HASH");
                break;
            case SHT_DYNAMIC:
                strcpy(typeOfTheFile, "DYNAMIC");
                break;
            case SHT_NOTE:
                strcpy(typeOfTheFile, "NOTE");
                break;
            case SHT_NOBITS:
                strcpy(typeOfTheFile, "NOBITS");
                break;
            case SHT_REL:
                strcpy(typeOfTheFile, "REL");
                break;
            case SHT_SHLIB:
                strcpy(typeOfTheFile, "SHLIB");
                break;
            case SHT_DYNSYM:
                strcpy(typeOfTheFile, "DYNSYM");
                break;
            case SHT_INIT_ARRAY:
                strcpy(typeOfTheFile, "INIT_ARRAY");
                break;
            case SHT_FINI_ARRAY:
                strcpy(typeOfTheFile, "FINI_ARRAY");
                break;
            case SHT_PREINIT_ARRAY:
                strcpy(typeOfTheFile, "PREINIT_ARRAY");
                break;
            case SHT_GROUP:
                strcpy(typeOfTheFile, "GROUP");
                break;
            case SHT_SYMTAB_SHNDX:
                strcpy(typeOfTheFile, "SYMTAB_SHNDX");
                break;
            case SHT_NUM:
                strcpy(typeOfTheFile, "NUM");
                break;
            case SHT_LOOS:
                strcpy(typeOfTheFile, "LOOS");
                break;
            case SHT_GNU_ATTRIBUTES:
                strcpy(typeOfTheFile, "GNU_ATTRIBUTES");
                break;
            case SHT_GNU_HASH:
                strcpy(typeOfTheFile, "GNU_HASH");
                break;
            case SHT_GNU_LIBLIST:
                strcpy(typeOfTheFile, "GNU_LIBLIST");
                break;
            case SHT_CHECKSUM:
                strcpy(typeOfTheFile, "CHECKSUM");
                break;
            case SHT_LOSUNW:
                strcpy(typeOfTheFile, "LOSUNW");
                break;
            case SHT_SUNW_COMDAT:
                strcpy(typeOfTheFile, "SUNW_COMDAT");
                break;
            case SHT_SUNW_syminfo:
                strcpy(typeOfTheFile, "SUNW_syminfo");
                break;
            case SHT_GNU_verdef:
                strcpy(typeOfTheFile, "GNU_verdef");
                break;
            case SHT_GNU_verneed:
                strcpy(typeOfTheFile, "GNU_verneed");
                break;
            case SHT_GNU_versym:
                strcpy(typeOfTheFile, "GNU_versym");
                break;
            case SHT_LOPROC:
                strcpy(typeOfTheFile, "LOPROC");
                break;
            case SHT_HIPROC:
                strcpy(typeOfTheFile, "HIPROC");
                break;
            case SHT_LOUSER:
                strcpy(typeOfTheFile, "LOUSER");
                break;
            case SHT_HIUSER:
                strcpy(typeOfTheFile, "HIUSER");
                break;

            default:
                strcpy(typeOfTheFile, "\0");
                break;
            }

            strTable = (char *)(elfFile->map_start + section[elfFile->header->e_shstrndx].sh_offset);
            if (idx >= 10)
            {
                printf("[ %d ]  %-25s  0x%-10X   0x%-10X    0x%-10X   %-10s\n",
                       idx, &strTable[section[idx].sh_name], sectionAddress, sectionOffset, sectionSize, typeOfTheFile);
            }
            else
            {
                printf("[ %d ]   %-25s  0x%-10X   0x%-10X    0x%-10X   %-10s\n",
                       idx, &strTable[section[idx].sh_name], sectionAddress, sectionOffset, sectionSize, typeOfTheFile);
            }
            printRequiredDataInDebugMode(idx, section, elfFile);
            idx++;
        }
    }
}

void printSymbols()
{
    //this operation displays information on all the symbol names in ELF file.
    for (int i = 0; i < numOfElfFiles; i++)
    {
        ElfFileInfo *elfFile = elfFiles[i];
        if (CurrentFileDirectory == -1)
        {
            perror("no file descriptor open!!!\n");
            _exit(EXIT_FAILURE);
        }

        int i;
        Elf32_Shdr *Symbols = (Elf32_Shdr *)(elfFile->map_start + elfFile->header->e_shoff);
        char *SectionHeaderStrTable = (char *)(elfFile->map_start + Symbols[elfFile->header->e_shstrndx].sh_offset);
        printf("\nSymbol Tables for %s ELF file :\n\n", elfFile->filename);

        for (i = 0; i < elfFile->header->e_shnum; i++)
        {
            if (Symbols[i].sh_type == SHT_SYMTAB || Symbols[i].sh_type == SHT_DYNSYM)
            {
                Elf32_Sym *sym_table_entry = (Elf32_Sym *)(elfFile->map_start + Symbols[i].sh_offset); // Section file offset
                int size = Symbols[i].sh_size / sizeof(Elf32_Sym);                                     //size of the section
                char *symbol_name = (char *)(elfFile->map_start + Symbols[Symbols[i].sh_link].sh_offset);

                if (debugMode)
                {
                    // printf("\nSymbol Table : %s\n", &SectionHeaderStrTable[sym_table_entry[i].st_name]);
                    printf("\nThe size of this symbol table: %d\n", Symbols[i].sh_size);
                    printf("The number of sybmols in this symbol table (number of enteries): %d\n", size);
                }
                printf("\n%s\t %s\t\t %s\t %s\t %s\n", "[Index]", "Value", "SectionIndex", "SectionName", "SymbolName");
                int j;
                for (j = 0; j < size; j++)
                {
                    int section = sym_table_entry[j].st_shndx;
                    bool validSection = true;
                    if (section == SHN_ABS || section == SHN_UNDEF)
                    {
                        validSection = false;
                    }
                    if (validSection)
                    {
                        printf("[ %d ]\t %08x\t %d\t\t %-20s %s\n", j, sym_table_entry[j].st_value, sym_table_entry[j].st_shndx,
                               SectionHeaderStrTable + Symbols[sym_table_entry[j].st_shndx].sh_name,
                               symbol_name + sym_table_entry[j].st_name);
                    }
                    else if (section == SHN_ABS)
                    {
                        printf("[ %d ]\t %08x\t ABS %-20s %s\n", j, sym_table_entry[j].st_value, "",
                               symbol_name + sym_table_entry[j].st_name);
                    }
                    else if (section == SHN_UNDEF)
                    {
                        printf("[ %d ]\t %08x\t UND %-20s %s\n", j, sym_table_entry[j].st_value, "",
                               symbol_name + sym_table_entry[j].st_name);
                    }
                }
            }
        }
    }
}

void checkFilesForMerge()
{
    if (numOfElfFiles != 2)
    {
        //checking that two files have been opened and mapped!
        printf("You should examine two files first!\n");
        return;
    }
    Elf32_Shdr *firstSectionHeader = thereIsOneSymtab(elfFiles[0]);
    Elf32_Shdr *secondSectionHeader = thereIsOneSymtab(elfFiles[1]);
    if (firstSectionHeader == NULL || secondSectionHeader == NULL)
    {
        printf("feature not supported..more than one symtab!!\n");
        return;
    } //now we have two examine file, each one with exactly one symbol table
    //First ELF file:
    Elf32_Shdr *section1 = (Elf32_Shdr *)(elfFiles[0]->map_start + elfFiles[0]->header->e_shoff);
    int symtab1Size = firstSectionHeader->sh_size / firstSectionHeader->sh_entsize;
    char *fisrtStrtab = (char *)(elfFiles[0]->map_start + section1[firstSectionHeader->sh_link].sh_offset);
    Elf32_Sym *firstSymtab = (Elf32_Sym *)(elfFiles[0]->map_start + firstSectionHeader->sh_offset);

    //Second ELF file:
    Elf32_Shdr *section2 = (Elf32_Shdr *)(elfFiles[1]->map_start + elfFiles[1]->header->e_shoff);
    int symtab2Size = secondSectionHeader->sh_size / secondSectionHeader->sh_entsize;
    char *secondStrtab = (char *)(elfFiles[1]->map_start + section2[secondSectionHeader->sh_link].sh_offset);
    Elf32_Sym *secondSymtab = (Elf32_Sym *)(elfFiles[1]->map_start + secondSectionHeader->sh_offset);

    //now,for each ELF file to loop over all symbols in its symbol table except symbol 0

    //Check the first table
    int i = 1;
    while (i < symtab1Size)
    {
        switch (firstSymtab[i].st_shndx)
        {
        case SHN_UNDEF: //sym is undefined.. it should be defined in the second elf file symbol table..otherwise to exit
            checkIfSymtabIsUndefined(secondSymtab, symtab2Size, secondStrtab, fisrtStrtab + firstSymtab[i].st_name);
            //if its also undefined in the second ELF file, to exit
            break;
        default: //its defined.. so to check the other elf file symtab
            checkIfSymtabIsDefined(secondSymtab, symtab2Size, secondStrtab, fisrtStrtab + firstSymtab[i].st_name);
            //if its also defined in the second ELF file, to exit
            break;
        }
        i++;
    }

    //Check the second table
    i = 1;
    while (i < symtab2Size)
    {
        switch (secondSymtab[i].st_shndx)
        {
        case SHN_UNDEF:
            checkIfSymtabIsUndefined(firstSymtab, symtab1Size, fisrtStrtab, secondStrtab + secondSymtab[i].st_name);
            break;
        default:
            checkIfSymtabIsDefined(firstSymtab, symtab1Size, fisrtStrtab, secondStrtab + secondSymtab[i].st_name);
            break;
        }
        i++;
    }
}

void mergeELFfiles()
{
    /*some helpful information (from the assignment tasks):*/

    //we will merge two files even if checking to merge failed to pass
    //we need to modify only "e_shoff" field
    //we will Use a copy of the section header table of the first ELF file as an initial version of the section header table for the merged file.
    //To modify the "sh_off" and "sh_size" fields in each section header
    //merged section has a size that is the sum of the sizes (should accordingly change the appropriate section header)
    //no need to merge them in memory!

    if (numOfElfFiles != 2)
    {
        printf("You should examine two files !!\n");
        return;
    }
    FILE *mergedElfFile = fopen("out.ro", "wb");

    //first ELF file
    Elf32_Shdr *firstSectionHeader = thereIsOneSymtab(elfFiles[0]);
    void *firstMapStart = elfFiles[0]->map_start;
    Elf32_Ehdr *firstHeader = elfFiles[0]->header;
    Elf32_Shdr *firstShdr = (Elf32_Shdr *)(firstMapStart + firstHeader->e_shoff);
    char *fisrtShstrtab = (char *)(firstMapStart + firstShdr[firstHeader->e_shstrndx].sh_offset);

    //second ELF file
    Elf32_Shdr *secondSectionHeader = thereIsOneSymtab(elfFiles[1]);
    void *secondMapStart = elfFiles[1]->map_start;
    Elf32_Ehdr *secondHeader = elfFiles[1]->header;
    Elf32_Shdr *secondShdr = (Elf32_Shdr *)(secondMapStart + secondHeader->e_shoff);
    char *secondShstrtab = (char *)(secondMapStart + secondShdr[secondHeader->e_shstrndx].sh_offset);

    //new merged ELF file
    //to copy an initial version of the ELF header
    int updSectionHeaders = fwrite((char *)firstMapStart, 1, 54, mergedElfFile);
    //Create an initial version of the section header table for the merged file by copying that of the first ELF file.
    char newSectionHeader[firstHeader->e_shnum * firstHeader->e_shentsize];
    memcpy(newSectionHeader, (char *)(firstShdr), firstHeader->e_shnum * firstHeader->e_shentsize);

    //Loop over the entries of the new section header table, and process each section according to its type
    printf("\nInfo about the Section Headers:\n");
    for (int i = 0; i < firstHeader->e_shnum; i++)
    {
        //in each iteration, the offset may be updated
        int newOffset = ftell(mergedElfFile);
        if (i == 0)
        {
            printf("\noffset: 0\n");
        }
        else
        {
            printf("section header name: %s\n", fisrtShstrtab + firstShdr[i].sh_name);
            printf("offset: %x\n", newOffset);
        }
        if (strcmp(fisrtShstrtab + firstShdr[i].sh_name, ".text") == 0 || strcmp(fisrtShstrtab + firstShdr[i].sh_name, ".data") == 0 || strcmp(fisrtShstrtab + firstShdr[i].sh_name, ".rodata") == 0)
        //these three sections are merged the same way
        {
            // get the section
            Elf32_Shdr *section;
            char *Name = fisrtShstrtab + firstShdr[i].sh_name;
            for (int j = 0; j < secondHeader->e_shnum; j++)
            {
                if (strcmp(Name, secondShstrtab + secondShdr[j].sh_name) == 0)
                {
                    section = secondShdr + j;
                }
            }

            updSectionHeaders = fwrite((char *)(firstMapStart + firstShdr[i].sh_offset), 1, firstShdr[i].sh_size, mergedElfFile);
            if (section != NULL)
            {
                updSectionHeaders = fwrite((char *)(secondMapStart + section->sh_offset), 1, section->sh_size, mergedElfFile);
                printf("Size = %x \n", ((Elf32_Shdr *)newSectionHeader)[i].sh_size + section->sh_size);
                //to update merged section header size
                ((Elf32_Shdr *)newSectionHeader)[i].sh_size += section->sh_size;
            }
        }

        else if (strcmp(fisrtShstrtab + firstShdr[i].sh_name, ".symtab") == 0)
        {
            char symbols[firstShdr[i].sh_size];
            memcpy(symbols, (char *)(firstMapStart + firstShdr[i].sh_offset), firstShdr[i].sh_size);
            Elf32_Shdr *SectionHeader1 = (Elf32_Shdr *)(elfFiles[0]->map_start + elfFiles[0]->header->e_shoff);
            Elf32_Shdr *SectionHeader2 = (Elf32_Shdr *)(elfFiles[1]->map_start + elfFiles[1]->header->e_shoff);
            int firstSymtabSize = firstSectionHeader->sh_size / firstSectionHeader->sh_entsize;
            int secondSymtabSize = secondSectionHeader->sh_size / secondSectionHeader->sh_entsize;
            char *firstStrtab = (char *)(elfFiles[0]->map_start + SectionHeader1[firstSectionHeader->sh_link].sh_offset);
            char *secondStrtab = (char *)(elfFiles[1]->map_start + SectionHeader2[secondSectionHeader->sh_link].sh_offset);
            Elf32_Sym *firstSymtab = (Elf32_Sym *)(elfFiles[0]->map_start + firstSectionHeader->sh_offset);
            Elf32_Sym *secondSymtab = (Elf32_Sym *)(elfFiles[1]->map_start + secondSectionHeader->sh_offset);
            for (int i = 1; i < firstSymtabSize; i++)
            {
                if (firstSymtab[i].st_shndx == SHN_UNDEF)
                //copying over symbol values and definition of symbols from the second ELF file
                {
                    for (int j = 1; j < secondSymtabSize; j++)
                    {
                        if (strcmp(firstStrtab + firstSymtab[i].st_name, secondStrtab + secondSymtab[j].st_name) == 0)
                        {
                            char *name = secondShstrtab + secondShdr[(secondSymtab + j)->st_shndx].sh_name;
                            ((Elf32_Sym *)symbols + i)->st_shndx = findNewIdx(firstShdr, firstSymtabSize, fisrtShstrtab, name);
                            ((Elf32_Sym *)symbols + i)->st_value = (secondSymtab + j)->st_value;
                        }
                    }
                }
            }

            updSectionHeaders = fwrite(symbols, 1, firstShdr[i].sh_size, mergedElfFile);
            printf("Size = %x \n", firstShdr[i].sh_size);
        }
        else
        {

            updSectionHeaders = fwrite((char *)(firstMapStart + firstShdr[i].sh_offset), 1, firstShdr[i].sh_size, mergedElfFile);
            printf("Size = %x \n", firstShdr[i].sh_size);
        }
        if (i != 0)
            ((Elf32_Shdr *)newSectionHeader)[i].sh_offset = newOffset;
        printf("\n");
    }

    printf("\nInfo about ELF Header:\n");

    int mergedShoff = ftell(mergedElfFile);
    updSectionHeaders = fwrite((char *)newSectionHeader, 1, firstHeader->e_shnum * firstHeader->e_shentsize, mergedElfFile);
    printf("\nStart of section headers: %d\n", mergedShoff);
    fseek(mergedElfFile, 32, SEEK_SET);
    updSectionHeaders = fwrite((char *)&mergedShoff, 4, 1, mergedElfFile);
    printf("number of section headers: %d\n", firstHeader->e_shnum);
    fclose(mergedElfFile);

    printf("\nInfo about the symbol table ... use readelf and compare!\n");
}

void quit()
{
    closeFileDirectory();
    unmapMemory();

    if (debugMode == 1)
    {
        printf("Quitting..\n");
    }

    exit(0);
}

/*HELPER FUNCTIONS:*/

//for Quit operation
void closeFileDirectory()
{
    if (!(CurrentFileDirectory == -1))
    {
        close(CurrentFileDirectory);
        CurrentFileDirectory = -1;
    }
}
void unmapMemory()
{
    for (int i = 0; i < numOfElfFiles; i++)
    {
        ElfFileInfo *elfFile = elfFiles[i];
        if (elfFile->map_start == NULL)
        {
            return; // if map_start is NULL then exit the function
        }

        if (munmap(elfFile->map_start, elfFile->fileDirectoryStat.st_size) < 0)
        {
            perror("munmap");
        }
    }
}

//for examineElfFile operation
void printRequiredData(ElfFileInfo *elfFile)
{
    int data_encoding;

    printf("\ninformations from the ELF Header:\n");
    if (!(elfFile->header->e_ident[EI_MAG0] == ELFMAG0))
    {
        printf("error in MAGIC NUM into file!\n");
        quit();
    }
    else if (!(elfFile->header->e_ident[EI_MAG1] == ELFMAG1))
    {
        printf("error in MAGIC NUM into file!\n");
        quit();
    }
    else if (!(elfFile->header->e_ident[EI_MAG2] == ELFMAG2))
    {
        printf("error in MAGIC NUM into file!\n");
        quit();
    }
    else if (!(elfFile->header->e_ident[EI_MAG3] == ELFMAG3))
    {
        printf("error in MAGIC NUM into file!\n");
        quit();
    }

    else if (!(elfFile->header->e_ident[EI_CLASS] == ELFCLASS32))
    {
        printf("\nELF FILE IS NOT A 32 BIT ELF FILE TYPE!!!\n");
        quit();
    }

    printf("\nData for %s ELF file :\n", elfFile->filename);
    if (elfFile->header->e_ident[EI_MAG0] == ELFMAG0 && elfFile->header->e_ident[EI_MAG1] == ELFMAG1 && elfFile->header->e_ident[EI_MAG2] == ELFMAG2 && elfFile->header->e_ident[EI_MAG3] == ELFMAG3)
    {
        printf(" => Bytes 1,2,3 of the magic numbers: ");
        printf("  Byte 0: %X | ", elfFile->header->e_ident[EI_MAG0]);
        printf("  Byte 1: %X | ", elfFile->header->e_ident[EI_MAG1]);
        printf(" Byte 2: %X | ", elfFile->header->e_ident[EI_MAG2]);
        printf(" Byte 3: %X  \n", elfFile->header->e_ident[EI_MAG3]);
    }

    data_encoding = elfFile->header->e_ident[EI_DATA];
    switch (data_encoding)
    {
    case 0:
        printf(" => Data Encoding Scheme:               Invalid data encoding\n");
        break;
    case 1:
        printf(" => Data Encoding Scheme:               2's complement, little endian\n");
        break;
    case 2:
        printf(" => Data Encoding Scheme:               2's complement, big endian\n");
        break;

    default:
        break;
    }

    printf(" => Entry Point Address (in hexa):      0x%X\n", elfFile->header->e_entry);
    printf(" => File Offset of SectionHeader Table: %d (bytes into file)\n", elfFile->header->e_shoff);
    printf(" => The num Of Section Header Entries:  %d\n", elfFile->header->e_shnum);
    printf(" => Size Of Each Section Header Entry:  %d (bytes)\n", elfFile->header->e_shentsize);
    printf(" => Program Header Table Offset:        %d (bytes into file)\n", elfFile->header->e_phoff);
    printf(" => Number of Program Header Entries:   %d\n", elfFile->header->e_phnum);
    printf(" => Size Of Each Program Header Entry:  %d (bytes)\n\n", elfFile->header->e_phentsize);
}

//for printSectionNames operation
void printRequiredDataInDebugMode(int idx, Elf32_Shdr *section, ElfFileInfo *elfFile)
{
    if (debugMode == 1)
    {
        printf("\nshstrndx = 0x%-27X", elfFile->header->e_shstrndx);
        printf("section name offset = 0x%X\n\n", section[idx].sh_name);
    }
}

//for checkFilesForMerge operation
Elf32_Shdr *thereIsOneSymtab(ElfFileInfo *info)
{
    Elf32_Shdr *shdr = (Elf32_Shdr *)(info->map_start + info->header->e_shoff);
    Elf32_Shdr *section = NULL;
    int counter = 0;
    int i = 0;
    while (i < info->header->e_shnum)
    {
        if (shdr[i].sh_type == SHT_SYMTAB || shdr[i].sh_type == SHT_DYNSYM)
        {
            section = &shdr[i];
            counter++;
        }
        i++;
    }
    if (counter != 1)
    {
        section = NULL;
    }
    return section;
}
void checkIfSymtabIsUndefined(Elf32_Sym *symtab, int sizeOfSymtab, char *strtab, char *symName)
{
    //to exit if its undefined
    int i = 1;
    while (i < sizeOfSymtab)
    {
        if (strcmp(symName, strtab + symtab[i].st_name) == 0)
        {
            if (symtab[i].st_shndx == SHN_UNDEF)
            {
                fprintf(stderr, "Symbol %s undefined\n", symName);
            }
            return;
        }
        i++;
    }
    printf("Symbol %s undefined\n", symName);
}
void checkIfSymtabIsDefined(Elf32_Sym *symtab, int sizeOfSymtab, char *strtab, char *symName)
{
    //to exit if its defined

    if (strcmp(symName, "") == 0)
    {
        return; // not section
    }
    int i = 1;
    while (i < sizeOfSymtab)
    {
        if (strcmp(symName, strtab + symtab[i].st_name) == 0)
        {
            if (symtab[i].st_shndx != SHN_UNDEF)
            {
                fprintf(stderr, "Symbol %s defined multiple times\n", symName);
            }
            return;
        }
        i++;
    }
}

//for mergeELFfiles operation
int findNewIdx(Elf32_Shdr *Shdr, int size, char *shstrtab, char *Name)
{
    int newIndex = SHN_UNDEF;
    int i = 0;
    while (i < size)
    {
        if (strcmp(shstrtab + Shdr[i].sh_name, Name) == 0)
        {
            newIndex = i;
            return newIndex;
        }
        i++;
    }
    return newIndex;
}//