#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <make_log.h>


int main(int argc, char *argv[])
{
	int fd[2];
	char buf[1024]={0};
	if(argc<2)
	{
		printf("useage:client filename\n");
	}
	int ret = pipe(fd);
	if(ret == -1)
	{
		perror("pipe");
		exit(1);
	}
	pid_t pid = fork();

	if(pid > 0)
	{
		// 父进程 -- 读
		// 关闭写端
		close(fd[1]);
		read(fd[0], buf, sizeof(buf));
		LOG("storage", "storage_client", buf);
		close(fd[0]);
		wait(NULL);
	}
	else if(pid == 0)
	{
		// 子进程 -- 写
		// 关闭读端
		close(fd[0]);
		dup2(fd[1],STDOUT_FILENO);
		
		execlp("fdfs_upload_file","fdfs_upload_file","/etc/fdfs/client.conf",argv[1],NULL);
		close(fd[1]);
	}
	return 0;
}