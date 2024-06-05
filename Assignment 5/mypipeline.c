#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    int rw[2];// read-write

    if (pipe(rw) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "(parent_process>forking…)\n");
    pid_t p1 = fork();// create child 1

    fprintf(stderr, "(parent_process>created process with id: %d)\n", p1);

    if (p1 == 0) {// child 1
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");
        close(1);// close standart output
        dup(rw[1]);// duplicate write for child1
        close(rw[0]);// close read for child1
        close(rw[1]);// close the duplicated write for child1
        char* a1[] = {"ls", "-l", NULL};//arguments for execvp
        fprintf(stderr, "(child1>going to execute cmd: …)\n");
        if(execvp("ls", a1) == -1){//execvp
            perror("execvp");//error
            exit(1);
        }
    }
    else if (p1 == -1) {// error fork
        perror("fork");
        exit(1);
    } 
    else {// parent
        fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n");
        close(rw[1]);// close write for parent
    }

    fprintf(stderr, "(parent_process>forking…)\n");
    pid_t p2 = fork();// create child 2

    fprintf(stderr, "(parent_process>created process with id: %d)\n", p2);

    if (p2 == 0) {// child 2
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");
        close(0);// close standart input
        dup(rw[0]);// duplicate read for child 2
        close(rw[1]);// close writ efor child 2
        close(rw[0]);// close duplicated read for child 2
        char* a2[] = {"tail", "-n", "2", NULL};//args for execvp
        fprintf(stderr, "(child2>going to execute cmd: …)\n");
        if(execvp("tail", a2) == -1){//execvp
            perror("execvp");//error
            exit(1);
        }
    }
    else if (p2 == -1) {// error fork
        perror("fork");
        exit(1);
    } 
    else {// parent
        fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");
        close(rw[0]);// close read for parent
    }
    fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
    waitpid(p1, NULL, 0);//wait for child 1 to terminate

    fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
    waitpid(p2, NULL, 0);//wait for child 2 to terminate

    fprintf(stderr, "(parent_process>exiting…)\n");
    return 0;
}