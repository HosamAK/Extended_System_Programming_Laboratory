#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "LineParser.h"
#include <fcntl.h>

void execute(cmdLine *pCmdLine){
    int x;
    pid_t p = fork();
    if(p == 0){//created child proccess
        if (pCmdLine->outputRedirect != NULL) {//change output
            int out = open(pCmdLine->outputRedirect, O_WRONLY|O_CREAT|O_TRUNC, 0644);
            dup2(out, STDOUT_FILENO);
            close(out);
        }
        if (pCmdLine->inputRedirect != NULL) {//change input
            int in = open(pCmdLine->inputRedirect, O_RDONLY);
            dup2(in, STDIN_FILENO);
            close(in);
        }

        if(execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1){//exec
            perror("Error in execvp\n");
            exit(1);
        }

    }
    else if(p < 0){//error in creating child proccess
        perror("Error in fork()\n");
        exit(1);
    }
    else if(pCmdLine->blocking){//blocking
        waitpid(p, &x, 0);
    }
}

int main(int argc, char **argv){
    int debug = 0;//0 - debug off, 1- debug on
    for(int i=1; i<argc; i++){
			if(strcmp(argv[i], "-d") == 0)
				debug = 1;
	}
    while(1){
        char buf[PATH_MAX];//buffer
        if(getcwd(buf, sizeof(buf)) != NULL)
            printf("Current working directory: %s\n", buf);
        else
            printf("getcwd error\n");
    
        char input[2048];
        if(fgets(input, sizeof(input), stdin) == NULL){
            printf("Null input\n");
            return 1;
        }
        cmdLine *pcmd = parseCmdLines(input);

        //QUIT
        if(strcmp(pcmd->arguments[0], "quit") == 0){
            freeCmdLines(pcmd);
            break;
        }

        //SUSPEND
        if(strcmp(pcmd->arguments[0], "suspend") == 0){
            if(kill(atoi(pcmd->arguments[1]), SIGTSTP) == 0){
                printf("Process %d suspended successfully\n", atoi(pcmd->arguments[1]));
            }else{
                fprintf(stderr, "Failed to suspend proccess %d\n", atoi(pcmd->arguments[1]));
            }
            continue;
        }
        //WAKE
        if(strcmp(pcmd->arguments[0], "wake") == 0){
            if(kill(atoi(pcmd->arguments[1]), SIGCONT) == 0){
                printf("Process %d has woke successfully\n", atoi(pcmd->arguments[1]));
            }else{
                fprintf(stderr, "Failed to wake proccess %d\n", atoi(pcmd->arguments[1]));
            }
            continue;
        }

        //KILL
        if(strcmp(pcmd->arguments[0], "kill") == 0){
            //printf("ID: %d", atoi(pcmd->arguments[1]));
            if(kill(atoi(pcmd->arguments[1]), SIGINT) == 0){
                printf("Process %d killed successfully\n", atoi(pcmd->arguments[1]));
            }else{
                fprintf(stderr, "Failed to kill proccess %d\n", atoi(pcmd->arguments[1]));
            }
            continue;
        }

        //CD
        if(strcmp(pcmd->arguments[0], "cd") == 0){
            if (chdir(pcmd->arguments[1]) == -1) {
                perror("Error change dirictory\n");
            }
            if(debug){
            fprintf(stderr, "PID: %d\n", getpid());
            fprintf(stderr, "Executing command: %s\n", pcmd->arguments[0]);
            }
            continue;
        }

        //DEBUG
        if(debug){
            fprintf(stderr, "PID: %d\n", getpid());
            fprintf(stderr, "Executing command: %s\n", pcmd->arguments[0]);
        }
        execute(pcmd);//exec
        freeCmdLines(pcmd);//release
    }

    return 0;
}