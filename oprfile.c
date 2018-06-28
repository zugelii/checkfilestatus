#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <fcntl.h>
void main()
{
	int i;
	unsigned char buf;
	int fd = open("t", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		perror("can not openfile");
	}
	else
	{
		printf("success fd = %d\n", fd);
		buf = 'a';
		for(i = 0; i < (1 << 17); i ++)
		{
		   if(-1 == write(fd, &buf, 1))
			{
				perror("write error");
				exit(1);
			}
		}

		close(fd);
	}
}
