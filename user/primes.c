#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void read_and_filter(int read_fd) {
    int prime, num;
    
    // 从管道读取第一个数，这个数是当前进程筛选的素数
    if (read(read_fd, &prime, sizeof(prime)) <= 0) {
        printf("read from pipe\n");
        exit(1);
    }
    printf("prime %d\n", prime);  // 输出素数

    // 创建新的管道
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        printf("pipe create error\n");
        exit(1);
    }

    // 等待下一个数字，判断是否需要创建新的进程
    int is_first_num = 1;
    //只要读出来不为空，那么管道中的内容就还没读完
    while (read(read_fd, &num, sizeof(num)) > 0) {
        if (num % prime != 0) { // 如果不是素数的倍数，传递到下一个管道
            // 检查是否到了结束的数字
            if (num == 35) {
                close(pipefd[0]);
                close(pipefd[1]);
                return;
            }
            if(is_first_num){
                // 为新的素数创建一个子进程
                if (fork() == 0) { // 子进程
                    close(read_fd); // 关闭旧的管道读端
                    close(pipefd[1]); // 关闭新管道的写端
                    read_and_filter(pipefd[0]); // 递归调用读取和筛选函数
                    exit(0);
                }
            }    
            is_first_num = 0; // 已创建新的进程，标记

            // 写入下一个管道
            if (write(pipefd[1], &num, sizeof(num)) == -1) {
                printf("pipe write error\n");;
                exit(1);
            }
            
        }
    }

    //处理完毕，关闭管道
    close(read_fd); // 关闭旧的管道读端
    close(pipefd[1]); // 关闭新管道的写端

    // 如果创建了新的进程，需要等待该进程结束
    if (!is_first_num) {
        wait(0);  // 等待新创建的子进程结束
    }
}

int
main(int argc, char *argv[])
{
    int pipefd[2];
    
    // 创建初始管道
    if (pipe(pipefd) == -1) {
        printf("pipe create error\n");
        exit(1);
    }
    
    // 创建初始子进程
    if (fork() == 0) { // 子进程
        close(pipefd[1]); // 关闭管道写端
        read_and_filter(pipefd[0]); // 子进程开始执行读取和筛选
        exit(0);
    } else { // 父进程
        close(pipefd[0]); // 关闭管道读端
        for (int i = 2; i <= 35; ++i) {
            if (write(pipefd[1], &i, sizeof(i)) == -1) {
                printf("pipe write error\n");;
                exit(1);
            }
        }
        close(pipefd[1]); // 写入完成后关闭管道写端
    }

  // 等待第一个子进程结束
    wait((int *) 0);
  exit(0);
}