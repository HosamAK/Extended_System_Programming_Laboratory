#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
int main(int argc, char **argv){
    pid_t p =fork();
    int rw[2];// read and write - 1/0
    if(pipe(rw) == -1){
        fprintf(stderr, "Error in pipe\n");
        return 1;
    }

    char buf[] = "Hello";
    if(p > 0){//parent
        close(rw[1]);//read
        read(rw[0], buf, sizeof(buf));//read sent
        printf("Message from child: %s\n", buf);


    } else if(p == 0){//child
        close(rw[0]);//write
        write(rw[1], buf, sizeof(buf));//write message

        return 0;

    }else{
        fprintf(stderr, "Failed fork");//failed
        return 1;
    }

    return 0;
}