#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/wait.h>
#include <fcntl.h>
#include <unordered_map>
#include <vector>

using namespace std;

vector<string> history;
char* getUserName(){
    char* username = getenv("USER");
    return username;
}
void printCurrentPath(){
    char cwd[10000];
    getcwd(cwd,sizeof(cwd));
    char* username = getUserName();
    printf("%s:%s:",username,cwd);
}
void getCurrentPath(){

}

void executeIOR(char **a ,char *b,int IOR){
    if(strcmp(a[0],"exit") == 0){
        printf("Exiting.... \n");
        exit(0);
    }
    if(strcmp(a[0],"cd")==0){
        printf("Path changed \n");
        chdir(a[1]);
    }
    else{
        int pid = fork();
        if(pid == -1){
            printf("Cannot fork the child \n");
            exit(1);
        }
        else if(pid==0){
            if(IOR == 1){
                int filedes = open(b, O_CREAT | O_WRONLY, 0777);
                if(filedes < 0){
                    cout << "Cannot open file" << endl;
                }
                dup2(filedes, STDOUT_FILENO);
                close(filedes);
            }
            if(IOR == 2){
                int filedes = open(b, O_CREAT | O_WRONLY | O_APPEND, 0777);
                if(filedes < 0){
                    cout << "Cannot open file" << endl;
                }
                dup2(filedes, STDOUT_FILENO);
                close(filedes);

            }
            if(IOR == 3){
                int filedes = open(b, O_RDONLY, 0777);
                if(filedes < 0){
                    cout << "Cannot open file" << endl;
                }
                dup2(filedes, STDIN_FILENO);
                close(filedes);
            }
            if (execvp(a[0],a) < 0) { 
                printf("\nCould not execute command.. \n"); 
            }
            exit(0);
        }
        else {   
            waitpid(pid,NULL,0);   
        }
    }
}

void executeCommands(char **a ){
    if(strcmp(a[0],"exit") == 0){
        printf("Exiting.... \n");
        exit(0);
    }
    if(strcmp(a[0],"cd")==0){
        printf("Path changed \n");
        if(strcmp(a[1],"~")==0){
           /*  char cwd[10000];
            getcwd(cwd,sizeof(cwd)); */
            chdir("/home/vaibhav/Desktop/OS_Assignment_Shell");
        }
        else{
            chdir(a[1]);
        }
    }
    else if(strcmp(a[0],"history")==0){
        for(int i=0;i<history.size();i++){
            cout << history[i] << endl;
        }
    }
    else{
        int pid = fork();
        if(pid == -1){
            printf("Cannot fork the child \n");
            exit(1);
        }
        else if(pid==0){
            if (execvp(a[0],a) < 0) { 
                printf("\nCould not execute command.. \n"); 
            }
            exit(0);
        }
        else {   
            waitpid(pid,NULL,0);   
        }
    }
}

int parseString(char *inputString,char **answer){
    int pi=0;
    char *query=new char[strlen(inputString)];
    query = strtok(inputString,"|");
    while(query){
        answer[pi] = new char[strlen(query)];
        answer[pi] = query;
        query = strtok(NULL,"|");
        pi++;
    }
    answer[pi]=NULL;
    return pi;
}

void handleIOR(char *a){
    int IORTypee = 0;
    char *query=new char[strlen(a)];
    char *answer[100];
    char *IOR = new char[1];

    for(int i=0;i<strlen(a)-1;i++){
        if(a[i]=='>'){
            if(a[i+1]=='>'){
                IORTypee = 2;
            }
            else{
                IORTypee = 1;
            }
        }
        else if(a[i]=='<'){
            IORTypee = 3;
        }
        else{
        }
        if(a[strlen(a)-1]=='<'){
            IORTypee = 2;
        }
    }
    if(IORTypee == 1){
        strcpy(IOR, ">");
    }
    else if(IORTypee == 2){
        strcpy(IOR, ">>");
    }
    else if(IORTypee == 3){
        strcpy(IOR, "<");
    }
    else{
        cout << "There is no < here" << endl;
    }
    int pi=0;
    query = strtok(a,IOR);
    while(query){
        answer[pi] = new char[strlen(query)];
        strcpy(answer[pi],query);
        query = strtok(NULL,IOR);
        pi++;
    }

        if(IORTypee == 1 || IORTypee == 2 || IORTypee == 3){ // > is 1 and >> is 2 and < is 3
            int p2 = 0;
            char *answer2[100];
            char *query2 = new char(strlen(a));
            query2 = strtok(answer[0]," ");
            while(query2){
                answer2[p2] = new char[strlen(query2)];
                answer2[p2] = query2;
                query2 = strtok(NULL," ");
                p2++;
            }
            answer2[p2] = NULL;
            executeIOR(answer2,answer[1],IORTypee);
        }
        else{
            cout << "Invalid IO Redirection" << endl;
        }
}

void handlePipe(char **a, int pipeSize){
    int fd[pipeSize][2];
    int pid;
    for(int i=0;i<pipeSize;i++){
        pipe(fd[i]);
        if(fd[i] < 0){
            cout << "Cannot open file in pipe" << endl;
        }
    }
    for(int i=0;i<pipeSize;i++){
        char *query=new char[strlen(a[i])];
        char *answer[100];
        int pi=0;
        query = strtok(a[i]," ");
    
        while(query){
            answer[pi] = new char[strlen(query)];
            strcpy(answer[pi],query);
            query = strtok(NULL," ");
            pi++;
        }
        answer[pi] = NULL;

        pid = fork();
        if(pid == -1){
            printf("Cannot fork the child \n");
            exit(1);
        }
        else if(pid==0){
            if(i==0){
                close(STDOUT_FILENO);
                close(fd[i][0]);
                dup2(fd[i][1],STDOUT_FILENO);
            }
            else if(i==(pipeSize-1)){
                close(STDIN_FILENO);
                close(fd[i-1][1]);
                dup2(fd[i-1][0],STDIN_FILENO);
                
            }
            else{
                close(STDIN_FILENO);
                close(STDOUT_FILENO);
                dup2(fd[i-1][0], STDIN_FILENO);
                dup2(fd[i][1],STDOUT_FILENO);
                close(fd[i][0]);
            }
            if (execvp(answer[0],answer) < 0) { 
                printf("\nCould not execute command in pipe.. \n"); 
            }
        }
        else {   
            int stat;
            waitpid(pid,&stat,WUNTRACED);
        }
        close(fd[i][1]);
    }
    
}

void handleNormalExecution(char *arrayInput){
    int pi=0;
    char *query=new char[strlen(arrayInput)];
    char *answer[100];
    query = strtok(arrayInput," ");
    while(query){
        answer[pi] = new char[strlen(query)];
        answer[pi] = query;
        query = strtok(NULL," ");
        pi++;
    }
    answer[pi] = NULL;
    executeCommands(answer);
}

/* void setAlias(char* stringa){
    //vector<string> v;
    char** aa;
    int pi=0;
    char *query=new char[strlen(stringa)];
    query = strtok(stringa,"{}");
    while(query){
        aa[pi] = new char[strlen(query)];
        aa[pi] = query;
        cout << aa[pi] << " tokens" << endl;
        query = strtok(NULL,"{}");
        pi++;
    }
    /* char** aaa;
    int ppi =0;
    char *query2=new char[strlen(stringa)];
    for(int i=0;i<pi;i++){
        if((i%2)==0){
            query2 = strtok(aa[pi],"= ");
            while(query2){
                aaa[ppi] = new char[strlen(query2)];
                aaa[ppi] = query2;
                query2 = strtok(NULL,"= ");
                cout << aaa[ppi] << endl;
            }
        }
        else{
            aaa[ppi] = aa[i];
        }
        ppi++;
    } 
} */

int main() { 
    while(1) {
        string inputCommand="";
        int pipeFlag=0;
        int IORFlag=0;
        char *pmeteres[1000];
        
        printCurrentPath();
        getline(cin, inputCommand);
        //setAlias(inputCommand);

        for(int i=0;i<inputCommand.size();i++){
            if(inputCommand[i]=='|'){
                pipeFlag = 1;
            }
            if(inputCommand[i]=='<' || inputCommand[i]=='>'){
                IORFlag = 1;
            }
        }
        history.push_back(inputCommand);
        char *inputString = new char [inputCommand.size()+1];
        
        strcpy(inputString, inputCommand.c_str());
        cout << "after stringcopy" << endl;
        int pipeSize = parseString(inputString,pmeteres);
        cout << "Before alias" << endl;
        //setAlias(inputString);
        
        if(pipeFlag == 1){
            handlePipe(pmeteres,pipeSize);
        }
        else{
            if(IORFlag == 1){
                handleIOR(inputString);
            }
            else{
                handleNormalExecution(inputString);
            }
        }
    }
    return 0;
}