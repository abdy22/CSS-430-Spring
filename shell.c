/*
Program: Simple shell program
Author: Abdirahman Hassan
Date: 4/19/2021
*/
#include <stdio.h>
#include <assert.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>
#include <fcntl.h>

#define MAX_LINE 80

int numberOfHistory =0; // # of history
char *outputFile=NULL; // redirected ouput file
char *inputFile=NULL;  // redirected input file
bool pipeFlag=false;  // is true if | is seen else false
bool historyFlag=false; // 
bool twoCommands=false; // is true of & command is seen, else false 
char  history[MAX_LINE/2+1];
//******************************************************************
// reset the values of @inputFile , @outputFile , @pipeFlag and @twoCommands
// the values are reseted when the user commands are processed 
//************************************************************************
void reset(){
    if(inputFile!=NULL) inputFile=NULL;
    if(outputFile!=NULL) outputFile=NULL;
    if(pipeFlag==true) pipeFlag=false;
    if(twoCommands==true)twoCommands=false;
    if(historyFlag==true)historyFlag=false;

}
//*************************************************
//update the history
// update the history with the newHist
//*************************************************
void updateHistory( char newHis[]){
    int i=0;
    while(i< strlen(newHis)){
        history[i]=newHis[i];
       i++;
    }
}
//*************************************************
// checks if the user requested the history 
// if the user typed !!, return 1 
// else return -1; 
//*************************************************
int checkHistoryRequest(char inPut[]){
    if(strlen(inPut)==0) return -1;
    char check[MAX_LINE/2+1]; // temp array to copy the input, input would not be affected
    memcpy(check, inPut, strlen(inPut)+1);
    char *temp=strtok(check," ");
    //memcpy(history, val, strlen(val)+1);
     if(strcmp(temp, "!!" )==0) return 1;
     updateHistory(inPut);
     return -1;
}
//*************************************************
// Deletes the empty space at end val
//*************************************************
void delete_empty_space(char val[]){
    int i=0;
    while(val[i] !='\n' && val[i]!=';') 
       i++;
    val[i]='\0';   
    
}
//*************************************************
// reads the user commands into val
//*************************************************
int readInput(char val[]){
    if(fgets(val,MAX_LINE/2+1,stdin)!=NULL){
        if(strlen(val)!=0)
            delete_empty_space(val); // gets rid of the space at the end of the last char
        
        return 1;
        
    }
    
    return -1;
}

//*************************************************
// tokenize char array of val put into args 
// uses the delemiter of space
//************************************************* 
int processCommands( char val[], char *args[]){
    char *temp=strtok(val," ");
    int command=0;
    while(temp!=NULL){
         args[command]=temp;
         command++;
         temp=strtok(NULL," ");
        
       }
        args[command]=NULL; // make the last the poiter  a null, used by exec to know the end of the line
        return 1;
}
//*************************************************
// checks the if the user is requesting redirecting, piping or running two commands together
//*************************************************
int process_flags( char *commands[]){
    int i=0;
    while(commands[i]!=NULL){
        if(strcmp(commands[i], ">" )==0){
              outputFile=commands[i+1]; // the file for output 
              return i;
        }
        if(strcmp(commands[i], "<" )==0){ 
               inputFile=commands[i+1]; // the file for input 
               return i;
        }
        if(strcmp(commands[i], "|" )==0){ // pipe 
                pipeFlag=true;  //piping flag
                return i;    
        }
        if(strcmp(commands[i],"&")==0){
            twoCommands=true;  // the user is running to commands concurently 
            return i;
        }
        i++;
    }
    return i; // if the user is not redirecting, piping or running two commands together 
}
//*******************************************************************************************
//  process different arguments to be executed
// reads the user input and checks for flags of redirecting, piping or running two commands together
//*******************************************************************************************  

void processArgs(char *args[],char userInput[], char *pipes[], char *secondArgs[])
{
   int i=0;
   int proceeResult;
   char *tempArgs[MAX_LINE/2+1];
   int re= readInput(userInput); // get the user input
   if (re==-1) return;
   
   int his=checkHistoryRequest(userInput); // check history request
   if(his==1){ // if hitory is requested
        if(strlen(history)==0){ // if no history 
            printf("No History");
            return; 
            }
            printf("Running commmand: %s\n", history);
            char tempHistory[MAX_LINE/2+1];        
            memcpy(tempHistory, history, strlen(history)+1); // copies hisotry into tempHistory array so that history will be affected
            proceeResult=processCommands(tempHistory,tempArgs); //process history command
        }

    else {
        proceeResult=processCommands(userInput,tempArgs); // process the current user input with no !!
    }

    if(proceeResult==-1){  // empty inputs
            printf(" Error! can not process the command\n");

    }
    int location=process_flags(tempArgs); 
    while(i<location){
            args[i]=tempArgs[i];
            i++;
        }
    args[i]=NULL;
    i++; // skip of over & and |    for exampl | wc -l, if the current location is |, then skip
    if(pipeFlag==true){ // check if pipe flag is true
        int j=0;
        while(tempArgs[i]!=NULL){
            pipes[j]=tempArgs[i];
            i++;
            j++;
        }
        pipes[j]=NULL; //  wc -l null after the loop

    }
    if(twoCommands==true){ // if the user typed &  for exampple & whoami 
        int j=0;
        while(tempArgs[i]!=NULL){
        secondArgs[j]=tempArgs[i];
        i++;
        j++;
     }
     secondArgs[j]=NULL; // whomai NULL 
   }
  
}
////*************************************************
// handles the redirec request 
// checks if outputFile and inputFile is not null 
//*************************************************
void handleRedirect(){
    enum {READ, WRITE};                     // ReaD, WRite
   if(outputFile!=NULL){
      int fd=open(outputFile, O_CREAT |  O_WRONLY, 0777);
      dup2(fd, WRITE);
      close(fd);
    }
   if(inputFile!=NULL){
     int fd=open(inputFile, O_CREAT | O_RDONLY, 0777);
     dup2(fd, READ);
     close(fd);
   }

}
//*************************************************
// process pipes 
// child writing is args[] 
// reading is pipes[]
//*************************************************
void processPipe(char *pipes[],char *args[], int fd[]){
    enum {READ, WRITE};
    pid_t pid;
     if ( (pid = fork()) < 0){     
            perror("fork error");
            }
     if(pid==0){ // child
         dup2(fd[WRITE],STDOUT_FILENO); // write 
         close(fd[READ]);
         close(fd[WRITE]);
         int x =execvp(args[0],args);
         
         perror(args[0]);

     }
     else{ //parent
         dup2(fd[READ],STDIN_FILENO); //read
         close(fd[WRITE]);
         close(fd[READ]);
         execvp(pipes[0],pipes);
        perror(pipes[0]);
     }

}
//*************************************************
// main
//*************************************************
int main(void)
{
    enum {READ, WRITE};                     // ReaD, WRite
    pid_t pid;
    char *args[MAX_LINE/2+1];  //args for execution 
    char *pipes[MAX_LINE/2+1];  // piping args
    char *secondArgs[MAX_LINE/2+1]; // second arguments for concurent commands
    char  userInput[MAX_LINE/2+1]; //  user input
    bool should_run=true; 
    pid_t pidPipe[2];

    printf(" **Simple Shell** \n");
    printf(" To exit: control plus C\n");

    pipe(pidPipe);
    while(should_run){
        printf("osh>");
        fflush(stdout);
        processArgs(args,userInput,pipes,secondArgs);
        if(strlen(userInput)==0) continue;
        if(strcmp(userInput,"exit")==0) {
            should_run=false;
            continue;
        }
        
        if ((pid = fork()) < 0){
            fprintf(stderr, "Failed to fork()!\n");
            exit(2);
            }
        if (pid == 0){
            handleRedirect(); //redirect 
            
            if(pipeFlag==true){ //piping
                 pipe(pidPipe); 
                 processPipe(pipes,args,pidPipe); // pipes
            }
            if(twoCommands==true){ //two concurent commands
                pid_t pid2;  
                pid2=fork(); // second child
                if(pid2< 0){
                    fprintf(stderr, "Failed to fork()!\n");
                    exit(2);
                }
                if(pid2==0){ //second child
                    int ex=execvp(secondArgs[0],secondArgs); 
                    if(ex< 0){
                        printf(" Error with second argument\n");
                    }
                     return 0;
                }
                else{ //wait for the second child to terminate
                    int psre;
                    waitpid(pid2,&psre,0);
                  }
                } 
                // after the second child terminates, 
                // the first child is executed
                int ex=execvp(args[0],args);
                if(ex< 0){
                        printf("Error!! can not run the command\n");
                    }
                return 0;
            }
            else { // parent
                int psre;
                waitpid(pid,&psre,0);
                reset();
            }
    }
    printf("Ended ! \n");
    return 0;
                
    
}