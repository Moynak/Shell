#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


int notbreak=1;

int ch_dir(char * array)
{
    char ch =' ';
    char * temp = strchr(array, ch);
    while(*(temp+1)==' ')
    printf("Change Dir to -%s\n",temp);
    if (chdir(temp+1) != 0)
    {
        perror("chdir() failed :");
        return 1; 
    }  
    return 0;
}

int exiting()
{
    notbreak=0;
    printf("\nExiting...\n");
    exit(0);
    return 0;
}

int execute(char *array)
{
    int status=1;
    if(strcmp(array,"exit")==0){status =exiting();}
    if((array[0]=='c')&&(array[1]=='d')){status = ch_dir(array);return status;}
    pid_t pid;
    if((pid=fork())==0)
    {
        //execl("/bin/sh", "sh", "-c", array, (char *) NULL);
        char *token1[20];
        int cnt1=0;
        while((token1[cnt1]=strsep(&array," "))!=NULL){cnt1++;}
        if(execvp(token1[0],token1)==-1)
        {perror("EXEC :");exit(2);}
    }
    else{
        int stat = 0;
        pid_t res = waitpid(pid, &stat, 0);
        status =WEXITSTATUS(stat);
        if(status!=0){return 1;}
        else{return 0;}
    }
    //if((status=system(array))!=0){return 1;}
    return 0;
}

int pipex(char *str[])
{
    printf("Pip arg ");
    int n=0;
    while(str[n]!=NULL){printf("%s | ",str[n]);n++;}
    int status;
    int count=0;
    pid_t pid1;
    if((pid1=fork())==0)
    {
        int pfd[n-1][2];
        int c = pipe(pfd[0]);
        if(c==-1)
        {
            printf("Error in pipe formation\n");
            exit(1);
        }
        int pid = fork();
        count=1;
        if(pid ==0)
        {
            //printf("Inside child\n");
            while(count<n-1)
            {
                c = pipe(pfd[count]);
                if(c==-1)
                {
                    printf("Error in pipe formation\n");
                    exit(1);
                }
                if(fork()!=0)
                {
                    wait(NULL);
                    //read from this
                    close(pfd[count][1]);
                    dup2(pfd[count][0],0);
                    close(pfd[count][0]);
                    //write to this
                    close(pfd[count-1][0]);
                    dup2(pfd[count-1][1],1);
                    close(pfd[count-1][1]);

                    char *token2[20];
                    int cnt2=0;
                    while((token2[cnt2]=strsep(&str[n-1-count]," "))!=NULL){cnt2++;}
                    if(execvp(token2[0],token2)==-1)
                    {perror("PIP2 :");exit(2);}
                }
                count++;
            }
            //write to pfd[n-2]
            close(pfd[count-1][0]);
            dup2(pfd[count-1][1],1);
            close(pfd[count-1][1]);
            //execvp(argv[i+1],argv);
            char *token1[20];
            int cnt1=0;
            while((token1[cnt1]=strsep(&str[n-1-count]," "))!=NULL){cnt1++;}
            if(execvp(token1[0],token1)==-1)
            {perror("PIP1 :");exit(2);}
        }
        else
        {
            wait(NULL);
            
            //printf("\nInside parent\n");
            close(pfd[0][1]);
            dup2(pfd[0][0],0);
            close(pfd[0][0]);
            //execvp(argv[i+1],argv);
            char *token2[20];
            int cnt2=0;
            while((token2[cnt2]=strsep(&str[n-1]," "))!=NULL){cnt2++;}
            if(execvp(token2[0],token2)==-1)
            {perror("PIP2 :");exit(2);}
        }
    }
    else
    {
        /* code */
        int stat = 0;
        pid_t res = waitpid(pid1, &stat, 0);
        status =WEXITSTATUS(stat);
        if(status!=0){return 1;}
        else{return 0;}
    }
    return 0;
}


int checkOR(char * array)
{
    int i,c;c=0;
    for(i=0;i<strlen(array);i++)
    {
        if(array[i]=='|'){c++;if(c>2){printf("Incorrect 0 format\n");notbreak=0;return 1;}}
        if(array[i]!='|'){c=0;}
    }
    if(c>0){printf("Incorrect 0.5 format\n");notbreak=0;return 1;}
    char *buffer,*string,*found;
    buffer = strdup(array);
    char* pip[20];
    char* token[20];
    int cnt=0;
    int status =1;
    char *temp;
    while((token[cnt]=strsep(&buffer,"|"))!=NULL)
    {
        temp=strsep(&buffer,"|");
        if((temp==NULL)||(*temp=='\0')){
            status = execute(token[cnt]);
            if(temp==NULL){return status;}
            if (status==0){
                temp=strsep(&buffer,"|");
                if(temp==NULL){printf("Incorrect 1 format\n");notbreak=0;return 1;}
                return 0;}
        }
        else{
            //printf("Pip arg = %s | %s  \n",token[cnt],temp);
            pip[0]= token[cnt]; pip[1]=temp;
            //printf("Pip arg = %s | %s  \n",pip[0],pip[1]);
            //strcpy(pip[0],token[cnt]); strcpy(pip[1],temp);
            //printf("here\n");
            i=2;
            while(((temp = strsep(&buffer,"|"))!=NULL)&&(*temp!='\0'))
            {
                pip[i]=temp;i++;
            }
            pip[i]=NULL;
            //printf("Pip arg = %s | %s\n",pip[i-1],pip[i]);
            status = pipex(pip);
            if (status==0){return 0;}
            // else{
            //     temp=strsep(&buffer,"|");
            //     if((temp!=NULL)&&(*temp!='\0')){printf("Incorrect 2 format\n");notbreak=0;return 1;}
            //     else if(temp==NULL){return status;}
            // }    
        }
        cnt++;
    }

    printf("Incorrect 3 format\n");notbreak=0;return 1;
}


int checkAND(char * array)
{
    int i,c;c=0;
    for(i=0;i<strlen(array);i++){
        if(array[i]=='&'){c++;if(c>2){printf("Incorrect 4 format\n");notbreak=0;return 1;}}
        if(array[i]!='&'){c=0;}
    }
    if(c>0){printf("Incorrect 4.5 format\n");notbreak=0;return 1;}
    char *buffer,*string,*found;
    buffer = strdup(array);
    char* token[20];
    int cnt=0;
    int status =1;
    char *temp;
    while((token[cnt]=strsep(&buffer,"&"))!=NULL)
    {
        temp=strsep(&buffer,"&");
        if((temp==NULL)||(*temp=='\0')){
            status = checkOR(token[cnt]);
            if((status==1)||(notbreak==0)){return 1;}
            if(temp==NULL){return status;}
        }
        else{
            printf("Incorrect 5 format\n");notbreak=0;return 1;  
        }
        cnt++;
    }
    printf("Incorrect 7 format\n");notbreak=0;
    return 1;
}


int checkS(char * array)
{
    int i,c;c=0;
    for(i=0;i<strlen(array);i++){
        if(array[i]==';'){c++;if(c>1){printf("Incorrect 6.55 format\n");notbreak=0;return 1;}}
        if(array[i]!=';'){c=0;}
    }
    //if(c>0){printf("Incorrect 6.5 format\n");notbreak=0;return 1;}
    char *buffer,*string,*found;
    buffer = strdup(array);
    char* token[20];
    int cnt=0;
    int status =1;
    char *temp;
    while((token[cnt]=strsep(&buffer,";"))!=NULL)
    {
        if(notbreak==0){return 1;}
        temp=strsep(&buffer,";");
        if(temp==NULL){status = checkAND(token[cnt]);return status;}
        if(*temp=='\0'){status = checkAND(token[cnt]);return status;}
        else{
             status = checkAND(token[cnt]);
             status = checkAND(temp); 
        }
        cnt++;
    }
    //printf("Incorrect 8 format\n");notbreak=0;
    return 1;
}


int main(int argc, char* argv[])
{
    do
    {
        /* code */
        char s[100]; 
        printf("%smoynak@moynak-HP-Notebook:%s$%s",KCYN,getcwd(s, 100),KNRM);
        char buffer[1024];        
        int c,pos;
        pos=0;
        while(((c=getchar())!=EOF)&&(c!='\n'))
        {
            buffer[pos]=c;
            pos++;
        }
        buffer[pos]='\0';
        printf("buffer =%s\n",buffer);
        int i=0;c=0;int j=0;int fl=0;
        char nospace[strlen(buffer)];
        while(i<strlen(buffer))
        {
            if(buffer[i]==' '){c++;if((c<2)&&(fl==0)){nospace[j]=buffer[i];j++;}}
            else if((buffer[i]=='|')||(buffer[i]==';')||(buffer[i]=='&'))
            {
                if(c>0){nospace[j-1]=buffer[i]; c=0;}
                else{nospace[j]=buffer[i];j++;}
                if(buffer[i+1]==' '){fl=1;}
            }
            else{c=0;fl=0;nospace[j]=buffer[i];j++;}
            i++;
        }
        nospace[j]='\0';
        printf("buffer =%s\n",nospace);
        printf("Status = %d\n",checkS(nospace));
        if(notbreak==0){notbreak=1;}
        // if((status=system(buffer))!=0)
        // {
        //     printf("stat = %d\n",status);
        //     perror("Sys:");
        // }
        // printf("stat = %d\n",status);
    } while (notbreak);
    
    return 0;
}