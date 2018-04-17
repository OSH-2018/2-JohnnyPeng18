#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
            
extern char **environ;

int main() {
    /* 输入的命令行 */
    char cmd[256];
    /* 命令行拆解成的各部分，以空指针结尾 */
    char *args[128];
    while (1) {
        /* 提示符 */
        printf("# ");
        fflush(stdin);
        fgets(cmd, 256, stdin);
        /* 清理结尾的换行符 */
        int i;
        int t1=0,t2=0;
        for (i = 0; cmd[i] != '\n'; i++)
            ;
        cmd[i] = '\0';
        /* 拆解命令行 */
        args[0] = cmd;
        for (i = 0; *args[i]; i++)
            for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++)
                if (*args[i+1] == ' ') {
                    while (1){
                        if(*args[i+1] == ' '){
                            *args[i+1] = '\0';
                            args[i+1]++;
                        }
                        else{
                            break;
                        }
                    }
                    break;
                }
        args[i] = NULL;

        /* 没有输入命令 */
        if (!args[0]){
            printf("Empty command!\n");
            continue;
        }

        /*管道*/
        int cnt=0;
        int bp[2];
        char *argspipe1[128];
        char *argspipe2[128];
        /*检测管道重数*/
        for (t1=0;t1<i;t1++){
            if(strcmp(args[t1],"|") == 0){
                if(cnt==0){
                    bp[0]=t1;
                }
                else if(cnt==1){
                    bp[1]=t1;
                }
                cnt++;
            }
        }
        if(cnt==1){
            for (t2=0;t2<bp[0];t2++){
                argspipe1[t2]=args[t2];
            }
            argspipe1[bp[0]]=NULL;
            for (t2=bp[0]+1;t2<=i;t2++){
                argspipe2[t2-bp[0]-1]=args[t2];
            }
            int pipefd[2];
            if(pipe(pipefd) != 0){
                printf("%s\n",strerror(errno));
                continue;
            }
            pid_t pidpipe1;
            pid_t pidpipe2;
            if((pidpipe1=fork()) == 0){
                dup2(pipefd[1], STDOUT_FILENO); //输出重定向
                close(pipefd[1]);
                close(pipefd[0]);
                execvp(argspipe1[0],argspipe1);
                /* execvp失败 */
                printf("%s\n",strerror(errno));
                return 255;
            }
            else{
                if((pidpipe2=fork()) == 0){
                    dup2(pipefd[0], STDIN_FILENO);
                    close(pipefd[0]);
                    close(pipefd[1]);
                    execvp(argspipe2[0],argspipe2);
                    /* execvp失败 */
                    printf("%s\n",strerror(errno));
                    return 255;
                }
                else{
                    close(pipefd[0]);
                    close(pipefd[1]);
                    wait(NULL);
                    wait(NULL);
                continue;
                }
        }
        }
        else if(cnt==2){
            char *argspipe3[128];
            for (t2=0;t2<bp[0];t2++){
                argspipe1[t2]=args[t2];
            }
            argspipe1[bp[0]]=NULL;
            for (t2=bp[0]+1;t2<bp[1];t2++){
                argspipe2[t2-bp[0]-1]=args[t2];
            }
            argspipe2[bp[1]]=NULL;
            for (t2=bp[1]+1;t2<i;t2++){
                argspipe3[t2-bp[1]-1]=args[t2];
            }
            int pipefd1[2],pipefd2[2];
            if(pipe(pipefd1) != 0 || pipe(pipefd2) != 0){
                printf("%s\n",strerror(errno));
                continue;
            }
            pid_t pipepid1,pipepid2,pipepid3;
            if((pipepid1=fork()) == 0){
                dup2(pipefd1[1], STDOUT_FILENO); //输出重定向
                close(pipefd1[1]);
                close(pipefd1[0]);
                close(pipefd2[1]);
                close(pipefd2[0]);
                execvp(argspipe1[0],argspipe1);
                /* execvp失败 */
                printf("%s\n",strerror(errno));
                return 255;
            }
            else{
                if((pipepid2=fork()) == 0){
                    dup2(pipefd1[0], STDIN_FILENO);
                    dup2(pipefd2[1],STDOUT_FILENO);
                    close(pipefd1[0]);
                    close(pipefd1[1]);
                    close(pipefd2[0]);
                    close(pipefd2[1]);
                    execvp(argspipe2[0],argspipe2);
                    /* execvp失败 */
                    printf("%s\n",strerror(errno));
                    return 255;
                }
                else{
                    if((pipepid3=fork()) == 0){
                        dup2(pipefd2[0],STDIN_FILENO);
                        close(pipefd1[0]);
                        close(pipefd1[1]);
                        close(pipefd2[0]);
                        close(pipefd2[1]);
                        execvp(argspipe3[0],argspipe3);
                        /* execvp失败 */
                        printf("%s\n",strerror(errno));
                        return 255;
                    }
                    else{
                        close(pipefd1[0]);
                        close(pipefd1[1]);
                        close(pipefd2[0]);
                        close(pipefd2[1]);
                        wait(NULL);
                        wait(NULL);
                        wait(NULL);
                        continue;
                    }
                }
        }
        }
        else if(cnt >= 3){
            printf("只支持二重或三重管道！\n");
            continue;
        }


        /*文件重定向*/
        int type=0;
        cnt=0;
        int flag2=0;
        for (i=0;args[i]!=NULL;i++){
            if(strcmp(args[i],">") == 0){
                type=1;
                cnt++;
            }
            else if (strcmp(args[i],"<") == 0){
                type=2;
                cnt++;
            }
            else if (strcmp(args[i],">>") == 0){
                type=3;
                cnt++;
            }
            }
        if(cnt>1){
            printf("只支持一次重定向！\n");
            continue;
        }
        else if (cnt == 1){
            char *argsredirect[128];
            char *filename;
            char filepath[256];
            filepath[0]='\0';
            int fd;
            for (i=0;args[i]!=NULL;i++){
                if(flag2==0 && strcmp(args[i],">")!=0 && strcmp(args[i],"<")!=0 && strcmp(args[i],">>")!=0 ){
                    argsredirect[i]=args[i];
                }
                else if(strcmp(args[i],">")==0 || strcmp(args[i],"<")==0 || strcmp(args[i],">>")==0 ){
                    argsredirect[i]=NULL;
                    flag2=1;
                }
                else if(flag2==1 && strcmp(args[i],">")!=0 && strcmp(args[i],"<")!=0 && strcmp(args[i],">>")!=0 ){
                    filename=args[i];
                }
            }
            pid_t redirectpid=fork();
            if(redirectpid <0){
                printf("%s\n",strerror(errno));
            }
            else if(redirectpid == 0){
                strcat(filepath,getcwd(NULL,0));
                strcat(filepath,"/");
                strcat(filepath,filename);
                if(type == 1){
                    fd=open(filepath,O_WRONLY | O_CREAT);
                    if(fd == -1){
                        printf("ERROR!\n");
                    }
                    dup2(fd,STDOUT_FILENO);
                    close(fd);
                    execvp(argsredirect[0],argsredirect);
                    /* execvp失败 */
                    printf("%s\n",strerror(errno));
                    return 255;
                }
                else if(type == 2){
                    fd=open(filepath,O_RDONLY);
                    if(fd == -1){
                        printf("ERROR!\n");
                    }
                    dup2(fd,STDIN_FILENO);
                    close(fd);
                    execvp(argsredirect[0],argsredirect);
                    /* execvp失败 */
                    printf("%s\n",strerror(errno));
                    return 255;
                }
                else if(type == 3){
                    fd=open(filepath,O_WRONLY | O_APPEND);
                    if(fd == -1){
                        printf("ERROR!\n");
                    }
                    dup2(fd,STDOUT_FILENO);
                    close(fd);
                    execvp(argsredirect[0],argsredirect);
                    /* execvp失败 */
                    printf("%s\n",strerror(errno));
                    return 255;
                }
            }
            else{
                wait(NULL);
                continue;
            }


        }

        /* 内建命令 */
        /*修改当前目录*/
        if (strcmp(args[0], "cd") == 0) {
            if (args[1]){
               if(chdir(args[1])!=0)
                    printf("%s\n",strerror(errno));
            }
            else{
                printf("ERROR: Please input the directory!\n ");
            }
            continue;
        }
        /*查看当前目录*/
        if (strcmp(args[0], "pwd") == 0) {
            char *wd = NULL;
            wd=getcwd(NULL,0);
            puts(wd);
            free(wd);
            continue;
        }
        /*查看所有环境变量*/
        if (strcmp(args[0], "env") == 0) {
            char **env = environ;
            while (*env){
                printf("%s\n",*env);
                env++;
            }
            continue;

        }
        /*环境变量命令*/
        if (strcmp(args[0], "export") == 0){
            /*输出所有环境变量*/
            if(args[1]==NULL){
                printf("Illegal command!\n");
            }
            else if(strcmp(args[1], "-p") == 0){
                if(args[2]==NULL){
                    char **env2 = environ;
                    while (*env2){
                        printf("%s\n",*env2);
                        env2++;
                    }
                }
                else{
                    printf("Illegal command!\n");
                }
            }
            /*删除环境变量*/
            else if(strcmp(args[1], "-n") == 0){
                if(unsetenv(args[2])!=0){
                    printf("%s\n",strerror(errno));
                }
            }
            /*定义或修改环境变量*/
            else{
                char name[128];
                char value[128];
                int flag;
                int j,k;
                j=0;
                k=0;
                flag=0;
                name[0]=0;
                value[0]=0;
                for(i=0;*(args[1]+i)!='\0';i++){
                    if(*(args[1]+i)=='='){
                        break;
                    }
                }
                for(j=0;*(args[1]+j)!='\0';j++){
                    if(j<i){
                        name[j]=*(args[1]+j);
                    }
                    else if(j>i){
                        value[k]=*(args[1]+j);
                        k++;
                    }
                }
                name[i]='\0';
                value[k]='\0';
                if(name[0]==0||value[0]==0){
                    printf("Illegal command!\n");
                }
                else{
                    if(setenv(name,value,1)!=0){
                        printf("%s\n",strerror(errno));
                    }
                }
            } 
            continue;
        }
        /*退出*/
        if (strcmp(args[0], "exit") == 0)
            return 0;

        /* 外部命令 */
        pid_t pid = fork();
        if (pid < 0){
            printf("%s\n",strerror(errno));
        }
        else if (pid == 0) {
            /* 子进程 */
            execvp(args[0], args);
            /* execvp失败 */
            printf("%s\n",strerror(errno));
            return 255;
        }
        /* 父进程 */
        wait(NULL);
    }
}