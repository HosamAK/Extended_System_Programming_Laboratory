#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "LineParser.h"
#include <fcntl.h>

typedef struct process{
    cmdLine* cmd;                     /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                       /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	          /* next process in chain */
} process;

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 20

process* procsList = NULL;
char* history[HISTLEN];
int newest = 0;
int oldest = 0;

char* getCommandFromHistory(int execMe) {
    if (execMe >= 1 && execMe <= HISTLEN && history[(newest - execMe + HISTLEN) % HISTLEN] != NULL) {
        return history[(newest - execMe + HISTLEN) % HISTLEN];
    }else{
        perror("Invalid history index.\n");
        return NULL;
    }
}

char* getLastCommand() {
    if (history[(newest - 1 + HISTLEN) % HISTLEN] == NULL) {
        return NULL;
    }else {
        return history[(newest - 1 + HISTLEN) % HISTLEN];
    }
}

char* getBeforeLastCommand() {
    if (history[(newest - 2 + HISTLEN) % HISTLEN] == NULL) {
        return NULL;
    }else {
        return history[(newest - 2 + HISTLEN) % HISTLEN];
    }
}
void printHistory() {
    int k = oldest;
    int inc = 1;
    while (history[k] != NULL && k != newest) {
        printf("%d %s\n", inc, history[k]);
        k = (k + 1) % HISTLEN;
        inc++;
    }
}

void addHistory(char* saveHis) {
    char* curr = malloc(strlen(saveHis) + 1);
    strcpy(curr, saveHis);
    if (history[newest] != NULL) {
        free(history[newest]);
    }
    history[newest] = curr;
    newest = (newest + 1) % HISTLEN;
    if (newest == oldest) {
        oldest = (oldest + 1) % HISTLEN;
    }
}

void updateProcessStatus(process* process_list, int pid, int status){
    if(process_list != NULL){
        process* curr = process_list;
        while(curr != NULL){
            if(curr->pid != pid){
                curr = curr->next;
            }else{
                curr->status = status;
                return;
            }
        }
    }
}

void deleteProcess(process **process_list, process *p) {
    if(*process_list != NULL){
        if (*process_list == p) {
            *process_list = (*process_list)->next;
            free(p);
        }else{
            process *before = *process_list;
            while (before->next != NULL && before->next != p) {
                before = before->next;
            }
            if (before->next != NULL) {
                before->next = before->next->next;
                free(p);
            } 
        }
    }
}

void updateProcessList(process **process_list){
    if(process_list != NULL){
        process *curr = *process_list;
        while (curr != NULL) {
            int st;
            int rs = waitpid(curr->pid, &st, WNOHANG);
            if (rs == curr->pid) {
                if (WIFSTOPPED(st)) {
                    updateProcessStatus(*process_list, curr->pid, SUSPENDED);
                } else if (WIFCONTINUED(st)) {
                    updateProcessStatus(*process_list, curr->pid, RUNNING);
                }else if (WIFEXITED(st)) {
                    updateProcessStatus(*process_list, curr->pid, TERMINATED);
                } else if(WIFSIGNALED(st)){
                    updateProcessStatus(*process_list, curr->pid, TERMINATED);
                }
            } else if (rs == -1) {
                curr = curr->next;
                continue;
            }
            curr = curr->next;
        }
    }
}

void killProcess(pid_t pid) {
    if (kill(pid, SIGINT) == 0) {
        printf("PID %d terminated.\n", pid);
    }
     updateProcessStatus(procsList, pid ,TERMINATED);
}

void suspendProcess(pid_t pid) {
    if (kill(pid, SIGTSTP) == 0) {
        printf("Process with PID %d suspended.\n", pid);
    }
    updateProcessStatus(procsList, pid ,SUSPENDED);
}

void wakeProcess(pid_t pid) {
    if (kill(pid, SIGCONT) == 0) {
        printf("Process with PID %d woken up.\n", pid);
    }
    updateProcessStatus(procsList, pid ,RUNNING);
}

process* helpfree(process* remove){
    process* next = remove->next;
        freeCmdLines(remove->cmd);
        free(remove);
        return next;
}

void freeProcessList(process** process_list){
    process* curr = *process_list;
    while(curr != NULL){
        helpfree(curr);
    }
    *process_list = NULL;
}

cmdLine* duplicatecmd(const cmdLine* pCmdLine)
{
    if (pCmdLine == NULL)
        return NULL;

    cmdLine* duplicate = (cmdLine*) malloc(sizeof(cmdLine));

    memcpy(duplicate, pCmdLine, sizeof(cmdLine));
    duplicate->outputRedirect = pCmdLine->outputRedirect ? strdup(pCmdLine->outputRedirect) : NULL;//duplicate output
    duplicate->inputRedirect = pCmdLine->inputRedirect ? strdup(pCmdLine->inputRedirect) : NULL;//duplicate input
    duplicate->next = NULL;

    for (int i = 0; i < pCmdLine->argCount; i++)//duplicate args
        duplicate->arguments[i] = strdup(pCmdLine->arguments[i]);

    return duplicate;
}

void addProcess(process** process_list, cmdLine* cmd, pid_t p) {
    process* newplist = (process*) malloc(sizeof(process));
    newplist->next = NULL;
    newplist->status = RUNNING;
    newplist->pid = p;
    newplist->cmd = duplicatecmd(cmd);
    if(*process_list != NULL){
        newplist->next = *process_list;
        *process_list = newplist;
    } else {
        *process_list = newplist;
    }
}

void printProcessList(process** process_list) {
    updateProcessList(process_list);
    process* curr = *process_list;
    printf("PID    Status    Command\n");
    while (curr != NULL) {
        printf("%d   ", curr->pid);
        printf("%s   ", curr->cmd->arguments[0]);
        switch (curr->status) {
            case TERMINATED:
                printf("   Terminated\n");
                deleteProcess(process_list, curr);
                break;
            case RUNNING:
                printf("   Running\n");
                break;
            case SUSPENDED:
                printf("   Suspended\n");
                break;
        }
        curr = curr->next;
    }
}

void execute(cmdLine *pCmdLine){
    int x;
    int piperw[2];//read-write

    if(pipe(piperw) == -1){//error pipe
        perror("pipe");
        exit(1);
    }
    pid_t p = fork();//create child
    if(p == 0){//child proccess
        if (pCmdLine->outputRedirect != NULL) {
            int out = open(pCmdLine->outputRedirect, O_WRONLY|O_CREAT|O_TRUNC, 0644);
            dup2(out, STDOUT_FILENO);
            close(out);
        }
        if (pCmdLine->inputRedirect != NULL) {
            int in = open(pCmdLine->inputRedirect, O_RDONLY);
            dup2(in, STDIN_FILENO);
            close(in);
        }
        if(execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1){
            perror("Error in execvp\n");
            exit(1);
        }
    }
    else if(p < 0){//error in creating child proccess
        perror("Error in fork()\n");
        exit(1);
    }
    else{
        if(pCmdLine->blocking){//blocking
            waitpid(p, &x, 0);
        }
        
    }addProcess(&procsList, pCmdLine, p);
}

void executePipe(cmdLine *pCmdLineP1, cmdLine *pCmdLineP2){
    int rw[2];// read-write

    if (pipe(rw) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t p1 = fork();// create child 1


    if (p1 == 0) {// child 1
        close(1);// close standart output
        dup(rw[1]);// duplicate write for child1
        close(rw[0]);// close read for child1
        close(rw[1]);// close the duplicated write for child1
        if (pCmdLineP1->inputRedirect != NULL) {
            int in = open(pCmdLineP1->inputRedirect, O_RDONLY);
            dup2(in, STDIN_FILENO);
            close(in);
        }
        if (pCmdLineP1->outputRedirect != NULL) {
           perror("error in p1 output(should be null)\n");
           exit(1);
        }
        
        if(execvp(pCmdLineP1->arguments[0], pCmdLineP1->arguments) == -1){//execvp
            perror("execvp");//error
            exit(1);
        }
    }
    else if (p1 == -1) {// error fork
        perror("fork");
        exit(1);
    } 
    else {// parent
        addProcess(&procsList, pCmdLineP1, p1);
        close(rw[1]);// close write for parent
    }

    pid_t p2 = fork();// create child 2

    if (p2 == 0) {// child 2
        close(0);// close standart input
        dup(rw[0]);// duplicate read for child 2
        close(rw[1]);// close writ efor child 2
        close(rw[0]);// close duplicated read for child 2
        if (pCmdLineP2->outputRedirect != NULL) {
            int out = open(pCmdLineP2->outputRedirect, O_WRONLY|O_CREAT|O_TRUNC, 0644);
            dup2(out, STDOUT_FILENO);
            close(out);
        }
        if (pCmdLineP2->inputRedirect != NULL) {
            perror("error in child 2 input(should be null)\n");
        }
        if(execvp(pCmdLineP2->arguments[0], pCmdLineP2->arguments) == -1){//execvp
            perror("execvp");//error
            exit(1);
        }
    }
    else if (p2 == -1) {// error fork
        perror("fork");
        exit(1);
    } 
    else {// parent
        addProcess(&procsList, pCmdLineP2, p2);
        close(rw[0]);// close read for parent
    }
    waitpid(p1, NULL, 0);//wait for child 1 to terminate

    waitpid(p2, NULL, 0);//wait for child 2 to terminate

}

int main(int argc, char **argv){
    int debug = 0;//0 - debug off, 1- debug on
    for(int i=1; i<argc; i++){
			if(strcmp(argv[i], "-d") == 0)
				debug = 1;
	}
    while(1){
        char buf[PATH_MAX];
        if(getcwd(buf, sizeof(buf)) != NULL)
            printf("Current working directory: %s\n", buf);
        else
            printf("getcwd error\n");
    
        char input[2048];
        if(fgets(input, sizeof(input), stdin) == NULL){
            printf("Null input\n");
            return 1;
        }
        addHistory(input);
        cmdLine *pcmd = parseCmdLines(input);

        if (strcmp(pcmd->arguments[0], "!!") == 0) {
            char* execMe = getBeforeLastCommand();
            if(execMe != NULL){
                pcmd = parseCmdLines(execMe);
            }else{
                perror("no previews command executed\n");
            }
            printHistory();
        }
        else if (pcmd->arguments[0][0] == '!') {
            char num1 = pcmd->arguments[0][1];
            int i;
            if(pcmd->arguments[0][2] != ' ' && pcmd->arguments[0][2] != '\0'){
                char num2 = pcmd->arguments[0][2];
                char* str = (char*) malloc(3 * sizeof(char));
                str[0] = num1;
                str[1] = num2;
                str[2] = '\0';
                i = atoi(str);
                free(str);
            }else{
                char* str = (char*) malloc(2 * sizeof(char));
                str[0] = num1;
                str[1] = '\0';
                i = atoi(str);
                free(str);
            }
            if (i > 0 && i <= HISTLEN && i <= newest) {
                pcmd = parseCmdLines(history[(newest - i -1) % HISTLEN]);
            }
            else {
                printf("Invalid history index.\n");
            }
}
        if(strcmp(pcmd->arguments[0], "quit") == 0){
            freeProcessList(&procsList);
            freeCmdLines(pcmd);
            break;
        }
        if (strcmp(pcmd->arguments[0], "history") == 0) {
            printHistory();
            continue;
        }
        if (strcmp(pcmd->arguments[0], "procs") == 0) {
            printProcessList(&procsList);
            continue;
        }
        if(strcmp(pcmd->arguments[0], "suspend") == 0){
            pid_t p = atoi(pcmd->arguments[1]);
            suspendProcess(p);
            continue;

        }else if(strcmp(pcmd->arguments[0], "wake") == 0){

            pid_t p = atoi(pcmd->arguments[1]);
            wakeProcess(p);
            continue;
        }else if(strcmp(pcmd->arguments[0], "kill") == 0){
            pid_t p = atoi(pcmd->arguments[1]);
            killProcess(p);
            continue;
        }
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
        if(debug){
            fprintf(stderr, "PID: %d\n", getpid());
            fprintf(stderr, "Executing command: %s\n", pcmd->arguments[0]);
        }
        if(pcmd->next != NULL){
            executePipe(pcmd, pcmd->next);
            continue;
        }
        else{
            execute(pcmd);
        }
        freeCmdLines(pcmd);
    }

    return 0;
}