#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

typedef struct link link;
struct link
{
    link *nextVirus;
    virus *vir;
};
typedef struct menu
{
    char *name;
    void (*function)();
} menu;

link *list = NULL;
//char *f_name = NULL;
virus *readVirus(FILE *f)
{
    virus *v = malloc(sizeof(virus));
    if (v == NULL)
    {
        printf("Failed to allocate v in heap\n");
        free(v);
        exit(1);
    }
    if (fread(&v->SigSize, sizeof(unsigned short), 1, f) != 1)
    {
        free(v);
        return NULL;
    }

    if (fread(v->virusName, sizeof(char), 16, f) != 16)
    {
        free(v);
        return NULL;
    }
    v->sig = malloc(v->SigSize);
    if (v->sig == NULL)
    {
        printf("Failed to allocate v.sig in heap\n");
        free(v);
        exit(1);
    }
    if (fread(v->sig, sizeof(unsigned char), v->SigSize, f) != v->SigSize)
    {
        free(v);
        return NULL;
    }
    return v;
}

void printVirus(virus *virus, FILE *output)
{
    fprintf(output, "virusName: %s\n", virus->virusName);
    fprintf(output, "SigSize: %d\n", virus->SigSize);
    fprintf(output, "sig: ");
    for (int i = 0; i < virus->SigSize; i++)
    {
        fprintf(output, "%02X ", virus->sig[i]);
    }
    fprintf(output, "\n\n");
}
//1b
void list_print(link *virus_list, FILE *f)
{
    link *newpointer = virus_list;
    if (newpointer->vir != NULL)
    {
        printVirus(newpointer->vir, f);
    }
    while (newpointer->nextVirus != NULL)
    {
        newpointer = newpointer->nextVirus;
        if (newpointer->vir != NULL)
            printVirus(newpointer->vir, f);
    }
}
link *list_append(link *virus_list, virus *data)
{
    link *newlist = (link *)malloc(sizeof(link));
    newlist->vir = data;
    newlist->nextVirus = NULL;
    if (virus_list != NULL)
    {
        link *Last = virus_list;
        while (Last->nextVirus != NULL)
            Last = Last->nextVirus;
        Last->nextVirus = newlist;
        return virus_list;
    }
    else
    {
        return newlist;
    }
}
void list_free(link *virus_list)
{
    while (virus_list != NULL)
    {
        link *curr = virus_list;
        virus_list = virus_list->nextVirus;
        free(curr);
    }
}
void load_signatures()
{
    char file_name[256];
    printf("please enter signature file :\n");
    if (fgets(file_name, sizeof(file_name), stdin) != NULL)
    {
        file_name[strcspn(file_name, "\n")] = '\0';
        FILE *f = fopen(file_name, "r");
        if (f == NULL)
        {
            printf("Failed to open file\n");
            list_free(list);
            exit(1);
        }
        char magicNumber[4];
        if (fread(magicNumber, sizeof(char), 4, f) == 0)
        {
            printf("Did not read magic number\n");
            fclose(f);
            list_free(list);
            exit(1);
        }
        if (memcmp(magicNumber, "VISL", 4) != 0)
        {
            printf("Incorrect magic number\n");
            fclose(f);
            list_free(list);
            exit(1);
        }
        virus *v = readVirus(f);
        while (v != NULL)
        {
            list = list_append(list, v);
            v = readVirus(f);
        }
        fclose(f);
    }
}

void print_signatures()
{
    if (list != NULL)
        list_print(list, stdout);
}
//1c
void detect_virus(char *buffer, unsigned int size, link *virus_list)
{
    link *cur = virus_list;
    while (cur != NULL)
    {
        for (int i = 0; i < size - cur->vir->SigSize + 1; i++)
        {
            if (memcmp(buffer + i, cur->vir->sig, cur->vir->SigSize) == 0)
            {
                printf("Virus found!\n");
                printf("The starting byte location in the suspected file: %d\n", i);
                printf("Virus name: %s\n", cur->vir->virusName);
                printf("Size of the virus signature: %d\n", cur->vir->SigSize);
            }
        }
        cur = cur->nextVirus;
    }
}
void detect_viruses(char *filename)
{
    const int s = 1024;
    char *buf = (char *)malloc(s * sizeof(char));
    if (filename != NULL)
    {
        FILE *f = fopen(filename, "r");
        if (f == NULL)
        {
            printf("Failed to open file\n");
            free(buf);
            list_free(list);
            exit(1);
        }
        size_t read = fread(buf, 1, s, f);
        if (read == 0)
        {
            printf("Failed to read from file\n");
            fclose(f);
            free(buf);
            list_free(list);
            exit(1);
        }
        detect_virus(buf, read, list);
        fclose(f);
        free(buf);
    }
    else
    {
        printf("Failed to read file name\n");
        free(buf);
        list_free(list);
        exit(1);
    }
}
//2
void neutralize_virus(char *fileName, int signatureOffset)
{
    FILE *file = fopen(fileName, "rb+");
    //to add check for the file??
    char fisrtSigByta = 0xC3;
    fseek(file, signatureOffset, SEEK_SET);
    fwrite(&fisrtSigByta, 1, 1, file); //reading the first byte of the signature
    fseek(file, 0, SEEK_SET);          //seek the beginning of the file
    fclose(file);
}
void detectVirus2(char *filename, char *buffer, unsigned int size, link *virus_list)
{

    link *cur = virus_list;
    while (cur != NULL)
    {
        for (int i = 0; i < size - cur->vir->SigSize + 1; i++)
        {
            if (memcmp(buffer + i, cur->vir->sig, cur->vir->SigSize) == 0)
            {
                neutralize_virus(filename, i);
            }
        }
        cur = cur->nextVirus;
    }
}
void fix_file(char *filename)
{
    const int s = 1024;
    char *buf = (char *)malloc(s * sizeof(char));
    if (filename != NULL)
    {
        FILE *f = fopen(filename, "r");
        if (f == NULL)
        {
            printf("Failed to open file\n");
            free(buf);
            list_free(list);
            exit(1);
        }
        size_t read = fread(buf, 1, s, f);
        if (read == 0)
        {
            printf("Failed to read from file\n");
            fclose(f);
            free(buf);
            list_free(list);
            exit(1);
        }
        detectVirus2(filename, buf, read, list);
        fclose(f);
        free(buf);
    }
    else
    {
        printf("Failed to read file name\n");
        free(buf);
        list_free(list);
        exit(1);
    }
}

void quit()
{
    list_free(list);
    exit(0);
}

int main(int argc, char **argv)
{
    char tmp[20];
    char *operations[] = {"Load signatures",
                          "Print signatures",
                          "Detect viruses",
                          "Fix file",
                          "Quit"};

    while (1)
    {
        printf("\nPlease choose a function (ctrl^D for exit):\n");
        for (int i = 0; i <= 4; i++)
        {
            printf("%d) %s\n", i + 1, operations[i]);
        }
        printf("option : ");
        if (fgets(tmp, 20, stdin) != NULL)
        {
            int op = tmp[0] - '0';
            switch (op)
            {
            case 1: load_signatures(); break;
            case 2: print_signatures(); break;
            case 3: detect_viruses(argv[1]); break;
            case 4: fix_file(argv[1]); break;
            case 5: quit(); break;
            default : printf("Not within bounds\n");
            }
        }
    }
}