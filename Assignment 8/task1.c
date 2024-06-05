#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

typedef struct
{
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    /*
   .
   .
   Any additional fields you deem necessary
  */
    //additional fields
    char display_mode;
} state;

struct fun_desc
{
    char *name;
    void (*fun)(state *s);
};

void toggleDebugMode(state *s);
void setFileName(state *s);
void setUnitSize(state *s);
void loadIntoMemory(state *s);
void toggleDisplayMode(state *s);
void memoryDisplay(state *s);
void saveIntoFile(state *s);
void memoryModify(state *s);
void quit(state *s);

char *map(char *array, int array_length, char (*f)(char))
{
    char *mapped_array = (char *)(malloc(array_length * sizeof(char)));
    int i=0;
    while(i<array_length){
        mapped_array[i] = f(array[i]);
        i++;
    }
    return mapped_array;
}

int main(int argc, char *argv[])
{
    state* s = (state *)malloc(sizeof(state));
    (*s).debug_mode = 0;   //default off
    (*s).display_mode = 0; //default off
    struct fun_desc menu[] = {{"Toggle Debug Mode", toggleDebugMode},
                              {"Set File Name", setFileName},
                              {"Set Unit Size", setUnitSize},
                              {"Load Into Memory", loadIntoMemory},
                              {"Toggle Display Mode", toggleDisplayMode},
                              {"Memory Display", memoryDisplay},
                              {"Save Into File", saveIntoFile},
                              {"Memory Modify", memoryModify},
                              {"Quit", quit},
                              {NULL, NULL}};

    while (true)
    {

        if ((*s).debug_mode == 1)
        { //debug on
            printf("Unit Size: %d\n", (*s).unit_size);
            printf("File Name: %s\n", (*s).file_name);
            printf("Memory Count: %d\n\n", (*s).mem_count);
        }
        printf("Please choose a function:\n");
        for (int i = 0; i < 9; i++)
        {
            printf("%d) %s\n", i, menu[i].name);
        }
        printf("Option : ");

        int op;
        if (scanf("%d", &op) == EOF)
        {
            printf("\n\n");
            return 0;
        }
        if (op < 0 || op > 8)
        {
            printf("Not within bounds\n");
            return 0;
        }
        printf("Within bounds\n");

        menu[op].fun(s);
        printf("\n");
    }
    free(s);

    return 0;
}

void toggleDebugMode(state *s) //0
{
    if ((*s).debug_mode == 0)
    {
        printf("debug flag now : on\n");
        (*s).debug_mode = 1;
    }
    else
    {
        printf("debug flag now : off\n");
        (*s).debug_mode = 0;
    }
}

void setFileName(state *s) //1.a
{
    char FileName[102];
    printf("write the file name: ");
    scanf("%s", &FileName);
    if (s->debug_mode)
    {
        printf("Debug: file name set to %s\n", FileName);
    }
    strcpy(s->file_name, FileName);
}

void setUnitSize(state *s) //2
{
    int nUnit_s;
    printf("Enter the unit size (1, 2, or 4): ");
    scanf("%d", &nUnit_s);
    if (nUnit_s == 1 || nUnit_s == 2 || nUnit_s == 4)
    {
        (*s).unit_size = nUnit_s;
        if ((*s).debug_mode == 1)
        {
            printf("Debug: set size to %d\n", (*s).unit_size);
        }
    }
    else
    {
        printf("invalid unit size!\n");
    }
}
void loadIntoMemory(state *s) //3
{

    if ((*s).file_name == "")
    {
        printf("file name is empty!\n");
        return;
    }
    FILE *f = fopen((*s).file_name, "r");
    if (f == NULL)
    {
        printf("failed to open file\n");
        return;
    }
    int loc, len;
    printf("write location in hexa: ");
    scanf("%X", &loc);

    printf("write length in decimal: ");
    scanf("%d", &len) ;

    if ((*s).debug_mode == 1)
    {
        printf("File Name: %s\n", (*s).file_name);
        printf("Location: %d\n", loc);
        printf("Length: %d\n", len);
    }

    fseek(f, loc, SEEK_SET);
    size_t bytesRead = fread((*s).mem_buf, (*s).unit_size, len, f);
    printf("Loaded %d units into memory\n", len);
    fclose(f);
}

void toggleDisplayMode(state *s) //4
{
    if ((*s).display_mode == 0)
    {
        printf("Display flag now on, hexadecimal representation\n");
        (*s).display_mode = 1;
    }
    else
    {
        printf("Display flag now off, decimal representation\n");
        (*s).display_mode = 0;
    }
}
void memoryDisplay(state* s) {

    printf("Enter address and length:\n");
    unsigned int add, len;
    scanf("%x %u", &add, &len);
    getchar();

    printf("%s\n", (s->display_mode) ? "Hexadecimal" : "Decimal");
    printf("%s\n", (s->display_mode) ? "===========" : "=======");

    unsigned int i;
    unsigned char* ptrmemory = (add == 0) ? s->mem_buf : &(s->mem_buf[add]);
    for (i = 0; i < len; i++) {
        unsigned int val = 0;
        memcpy(&val, ptrmemory + i * s->unit_size, s->unit_size);

        if (!s->display_mode)
            printf(dec_formats[s->unit_size - 1], val);
        else
            printf(hex_formats[s->unit_size - 1], val);
    }

    printf("\n");
}

void saveIntoFile(state *s) {//1.d
    if (strlen(s->file_name) == 0) {
        printf("File name not set. Please use option 1 to set the file name.\n");
        return;
    }

    FILE *f = fopen(s->file_name, "r+b");
    if (f == NULL) {
        printf("Failed to open the file for writing.\n");
        return;
    }

    unsigned int src_add, trg_loc;
    int len;

    printf("Please enter <source-address> <target-location> <length>\n");
    scanf("%x %x %d", &src_add, &trg_loc, &len);

    
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    if (trg_loc >= file_size) {
        printf("Error: Target location is greater than the file size.\n");
        fclose(f);
        return;
    }

    
    fseek(f, trg_loc, SEEK_SET);

    
    unsigned char *str_add;
    if (src_add == 0) {
        str_add = s->mem_buf;
    } else {
        str_add = (unsigned char *)(s->mem_buf + (src_add - 1));
    }

    
    size_t byte_w = fwrite(str_add, s->unit_size, len, f);
    fclose(f);

    if (s->debug_mode) {
        printf("Debug: Written %zu bytes to the file.\n", byte_w);
    }
}

void memoryModify(state *s) {//1.e
    printf("Please enter <location> <val>\n");
    unsigned int loc;
    unsigned int value;

    scanf("%x %x", &loc, &value);

    if (s->debug_mode) {
        printf("Location: 0x%x\n", loc);
        printf("Val: 0x%x\n", value);
    }

    unsigned int calc_off = s->unit_size * loc;


        for (int i = 0; i < s->unit_size; i++) {
            s->mem_buf[calc_off + i] = (value >> (8 * i)) & 0xFF;
        }
        printf("Memory modified successfully.\n");

}

void quit(state *s) //8
{
    if ((*s).debug_mode == 1)
    {
        printf("quitting\n");
        exit(0);
    }
    free(s);
    exit(0);
}