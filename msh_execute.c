//sh util/final_commit.sh
//one more comment for final comment might do extra credit
#include <msh.h>
#include <msh_parse.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h> /* sigaction and SIG* */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <msh_parse.c>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
static char * background [MSH_MAXBACKGROUND];
char * current = NULL;
pid_t foreground;
pid_t shell_pgid;
pid_t allpids[MSH_MAXBACKGROUND];
pid_t suspended[MSH_MAXBACKGROUND];
void printjobs(){

    //this function prints all the jobs
    for (int i = 0; i < MSH_MAXBACKGROUND; i++){
        if (suspended[i] != 0){
            printf("[%d] ./sleep %d\n",i, i);
        }
        
    }
}
void checkupdate(){
    //this function checks all background and removes the backgrounds (shifts elements if removed) + can list jobs
    for (int i = 0; i < MSH_MAXBACKGROUND; i++){
        if (suspended[i] != 0){
            int status;
            // printf("before nohang\n");
            pid_t pid = waitpid(suspended[i], &status, WNOHANG);
            // printf("after nohang\n");

             if (pid == -1) {
                perror("waitpid");
            } else if (pid > 0) {
                // Child process has exited
                // Remove the background job by shifting elements if needed
                for (int j = i; j < MSH_MAXBACKGROUND - 1; j++) {
                    suspended[j] = suspended[j+1];
                }
                suspended[MSH_MAXBACKGROUND - 1] = 0;

                i--;
            } else if (pid == 0){
                // Child process is still running
                // printf("Job %s is running in the background\n", background[i]->p[0].List[0]);
            }
        }
    }
}
   void
msh_execute(struct msh_pipeline *p)
{
    
	if (p == NULL) {
        return;
    }
    int value_if = 0;
    if(strcmp(p->p[0].List[0], "fg") == 0){
        if(p->p[0].List[1] != NULL){
            int x = atoi(p->p[0].List[1])-1;
            if (suspended[x] > 0){
                value_if = 1;
                kill(suspended[x],SIGCONT);
                suspended[x] = 0;
            }
            for (int i = 0; i < MSH_MAXBACKGROUND; i++){

                if(suspended[x] > 0){ 
                    // printf("now running in the foreground\n");
                    // kill(suspended[j], SIGCONT);
                    waitpid(suspended[x],NULL, 0);
                }
            }
            
            if (value_if == 0){
                // printf("No jobs available to run in the foreground\n");
            }
        }
        else{
            p->flag = 0;
            if (suspended[0] > 0){
                value_if = 1;
                kill(suspended[0],SIGCONT);
                suspended[0] = 0;
            }
            for (int i = 0; i < MSH_MAXBACKGROUND; i++){

                if(suspended[i] > 0){
                    // printf("now running in the foreground\n");
                    // kill(suspended[j], SIGCONT);
                    waitpid(suspended[i],NULL, 0);
                }
            }
            
            if (value_if == 0){
                // printf("No jobs available to run in the foreground\n");
            }
        }
        
    }
    else if(strcmp(p->p[0].List[0], "jobs") == 0){
            // checkupdate();
            printjobs();
    }
    else{

    
        int counter = p->pElements;
        int pipes[counter][2];
        int i, j;
        int value_if = 0;
    
        for ( i = 0; i < counter; i++) {
            struct msh_command *command = &p->p[i];

            if(strcmp(p->p[0].List[0], "bg") == 0){
                
                p->flag = 1;
                for (int i = 0; i < MSH_MAXBACKGROUND; i++){
                    if(suspended[i] > 0){
                        kill(suspended[i],SIGCONT);
                        suspended[i] = 0;
                        value_if = 1;
                    }
                }
                if (value_if == 0){
                    // printf("No jobs available to run in the background\n");
                }
                
            }
            
            
            else if(strcmp(p->p[0].List[0], "") == 0){//tried to fix empty

            }
            else if(strcmp(p->p[0].List[0], "exit") == 0 || strcmp(p->p[0].List[0], "EXIT") == 0 ){
                exit(EXIT_SUCCESS);
            }
            else if(strcmp(p->p[0].List[0], "cd") == 0){
                if (p->p[0].List[2] != NULL) {
                    fprintf(stderr, "cd: too many argument\n");
                }
                if (p->p[0].List[1] == NULL) {
                    fprintf(stderr, "cd: no argument\n");
                }
                else{
                    if (p->p[0].List[1][0] == '~'){
                        char * way = malloc(strlen(getenv("HOME")) + strlen(p->p[0].List[1]));
                        strcpy(way, getenv("HOME"));
                        // fprintf(stderr, "the current path %s\n", way);
                        strcat(way, &p->p[0].List[1][1]);
                        // fprintf(stderr, "the current path %s\n", way);
                        if(chdir(way)){
                            fprintf(stderr, "cd: no such directory\n");
                        }
                        free(way);
                    }
                    else{
                        if(chdir(p->p[0].List[1])){
                            fprintf(stderr, "cd: no such directory\n");
                        }
                    }
                }
            }
            else{
                if(i != counter-1){
                    pipe(pipes[i]);
                }
                pid_t pid = fork();
                command->pid= pid;
                if (pid == 0) { // we are in the child
                    
                    foreground = pid;
                    current = command->program;
                    if(p->symbol == 1){
                        
                        int filefd;
                        if(p->file_err){
                            filefd = open(p->file_err, O_WRONLY|O_CREAT|O_APPEND, 0700);
                            filefd = 0;
                        }

                        if(p->appended == 1){
                            filefd = open(p->file_out, O_WRONLY|O_CREAT|O_APPEND, 0700);
                        }
                        else{
                            filefd = open(p->file_out, O_WRONLY|O_CREAT, 0700);
                            remove(p->file_out);
                            filefd = open(p->file_out, O_WRONLY|O_CREAT, 0700);
                        }
                        
                        close(1);
                        dup2(filefd, STDOUT_FILENO);

                    }

                    if(p->symbol == 2){
                        int filefd;

                        if(p->file_out){
                            filefd = open(p->file_out, O_WRONLY|O_CREAT|O_APPEND, 0700);
                            filefd = 0;

                        }
                        if(p->appended == 1){
                            filefd = open(p->file_err, O_WRONLY|O_CREAT|O_APPEND, 0700);
                        }
                        else{
                            filefd = open(p->file_err, O_WRONLY|O_CREAT, 0700);
                            remove(p->file_err);
                            filefd = open(p->file_err, O_WRONLY|O_CREAT, 0700);
                        }
                        close(0);
                        dup2(filefd, STDERR_FILENO);
                    }
                    
                    if (i != 0) {
                        dup2(pipes[i - 1][0], STDIN_FILENO);
                    }
                    if (i < counter - 1) {
                        dup2(pipes[i][1], STDOUT_FILENO);
                    }
                    for (j = 0; j < counter - 1; j++) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                    
                    // printf("before execvp =%s\n",current);
                    waitpid(-1, NULL, 0);
                    execvp(command->List[0], command->List);
                    
                    perror("execvp");
                }
                else{
                    
                    allpids[i] = pid;
                    
                }
            }
        }
        
        
        for ( i = 0; i < counter - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    
        if (!msh_pipeline_background(p)){
            for ( i = 0; i < counter; i++) {
                waitpid(allpids[i], NULL, 0);
            }
            foreground = 0;
        }
        if (msh_pipeline_background(p)){
            for (i = 0 ; i < MSH_MAXBACKGROUND; i++){
                pid_t pid = waitpid(allpids[i], NULL, WNOHANG);
                if (pid>0){

                }
                //this is where i check if it is running in the background
                //if it is place it in the global array
                
            }
        }
    }
}


void handle_SIGTSTP(int p) {
    // printf("CTRL-Z Pressed\n");
    (void) p;
    // printf("backgorunds[0] = %s\n", current);
    if (foreground != 0 ) {
        // Send the SIGTSTP signal to the foreground process group
        kill(-foreground, SIGTSTP);

        // Add the suspended process group to the suspends array
        for (int i = 0; i < 16; i++) {
            if (suspended[i] == 0) {
                background[i] = current;
                suspended[i] = foreground;
                break;
            }
        }

        // Reset the foreground process group ID
        foreground = 0;
    }
    //set foreground to background
    
    for (int i = 0; i < MSH_MAXBACKGROUND; i++){
        if (allpids[i] > 0){
            for (int j = 0 ; j < MSH_MAXBACKGROUND; j++){
                if(suspended[j] == 0){

                    suspended[j] = allpids[i];
                    break;
                }
            }
            kill(allpids[i], SIGTSTP);
        }
    }
    // foreground = NULL;
    // signal(SIGTSTP, SIG_IGN);
    // fflush(stdout);
    // pid_t pid = getpid();
    // kill(pid, SIGTSTP);
    
}

void handle_sigint(int p) {
    (void) p;
    if (foreground!= 0){
        kill(-foreground, SIGINT);
    }
}

void setup_signal_handlers() {
    struct sigaction act, sigact;

    act.sa_handler = handle_sigint;
    sigact.sa_handler = handle_SIGTSTP;

    sigemptyset(&act.sa_mask);
    sigemptyset(&sigact.sa_mask);

    act.sa_flags = 0;
    sigact.sa_flags = 0;

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTSTP, &sigact, NULL);
}

void
msh_init(void)
{
    //SIGINT is for cntrl c
    //SIGTSTP is for cntrl z
    shell_pgid = getpgrp();
    setup_signal_handlers();
	return;
}

void
setup_signal(int signo, void (*fn)(int , siginfo_t *, void *))
{
    sigset_t masked;
    struct sigaction siginfo;
    //cntrl c
    sigemptyset(&masked);
    sigaddset(&masked, signo);
    siginfo = (struct sigaction) {
        .sa_sigaction = fn,
        .sa_mask      = masked,
        .sa_flags     = SA_SIGINFO
    };

    if (sigaction(signo, &siginfo, NULL) == -1) {
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }


}