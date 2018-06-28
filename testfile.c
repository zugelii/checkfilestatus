#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define TMP_DIR     "systeminfor"

int fork_write(unsigned char *d, char n)
{
    int fd;
    int i = 0;
    while(i < 20)
    {

 	 fd=open(TMP_DIR, O_RDWR | O_NOCTTY | O_EXCL, S_IRUSR | S_IWUSR);
         if(fd==-1){
          printf("file not found.\n");
          return -1;
         }
	lseek(fd, i, SEEK_SET);
	i += n;
        write(fd, d, 1);
        close(fd);
	sleep(1);
	
    }	
}

int main(int argc,char *aa[]){
    pid_t pid;
    int i = 10, status;
    unsigned char buffer;
    pid = fork();
    if(pid < 0)
    {
	perror("fail to fork");
	exit(1);
    }else if(pid == 0)
    {
	printf("the first pid \r\n");
	buffer = 'b';
	fork_write(&buffer, 2);
    }else
    {
	printf("the parent pid:%d \r\n", getpid());
	buffer = 'c';
	fork_write(&buffer, 3);
	if(wait(&status) == -1)
	{
	   perror("fail to wait");
	   exit(1);
	}
    }    
    return 1;
}
