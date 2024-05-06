#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  //定义文件描述符  
  int parent_fd[2];
  int child_fd[2];
  //定义进程号
  int pid;
  char buf[10];

    //创建两个管道 ，考虑异常情况 
    if (pipe(parent_fd) == -1 || pipe(child_fd) == -1) {
        printf("pipe");
        printf("pipe create error");
        exit(1);
    } 
  
    //创建子进程
    pid = fork();
    if (pid == -1) {
        printf("fork");
        exit(1);
    }

    if (pid == 0) {
    // 子进程
    close(parent_fd[1]); 
    close(child_fd[0]);  
    //从父进程中读端读取ping
    if (read(parent_fd[0], buf, 4)== -1) {
        printf("error parent read\n");
        exit(1);
    }

    printf("%d: received %s\n", getpid(), buf);     // 子进程输出
    
    // 在子进程写端写入pong
    if(write(child_fd[1], "pong", 4) == -1) {
        printf("error child write\n");
        exit(1);
    }   
  
   } else {
        // 父进程
        close(parent_fd[0]); 
        close(child_fd[1]);

        // 在父进程写端写入ping
        if (write(parent_fd[1], "ping", 4) == -1) {
            printf("error parent write");
            exit(1);
        }

        // 在子进程读端读出pong
        if (read(child_fd[0], &buf, 4) == -1) {
            printf("error child read");
            exit(1);
        }

        printf("%d: received %s\n", getpid(), buf);     // 父进程输出
    }

    close(parent_fd[0]);
    close(parent_fd[1]);
    close(child_fd[0]);
    close(child_fd[1]);
    exit(0);
}