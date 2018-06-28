#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define TMP_DIR     "/tmp/systeminfor"
#define EEPROM_DIR "/sys/bus/nvmem/devices/1-00500/nvmem"

char *ptr_cmp;
static void displayInotifyEvent(struct inotify_event *i)
{
	printf("    wd =%2d; i->mask:%x", i->wd, i->mask);
	if (i->cookie > 0)
		printf("cookie =%4d; ", i->cookie);

	printf("mask = ");
	if (i->mask & IN_ACCESS)
		printf("IN_ACCESS ");
	if (i->mask & IN_ATTRIB)
		printf("IN_ATTRIB ");
	if (i->mask & IN_CLOSE_NOWRITE)
		printf("IN_CLOSE_NOWRITE ");
	if (i->mask & IN_CLOSE_WRITE)
		printf("IN_CLOSE_WRITE ");
	if (i->mask & IN_CREATE)
		printf("IN_CREATE ");
	if (i->mask & IN_DELETE)
		printf("IN_DELETE ");
	if (i->mask & IN_DELETE_SELF)
		printf("IN_DELETE_SELF ");
	if (i->mask & IN_IGNORED)
		printf("IN_IGNORED ");
	if (i->mask & IN_ISDIR)
		printf("IN_ISDIR ");
	if (i->mask & IN_MODIFY)
		printf("IN_MODIFY ");
	if (i->mask & IN_MOVE_SELF)
		printf("IN_MOVE_SELF ");
	if (i->mask & IN_MOVED_FROM)
		printf("IN_MOVED_FROM ");
	if (i->mask & IN_MOVED_TO)
		printf("IN_MOVED_TO ");
	if (i->mask & IN_OPEN)
		printf("IN_OPEN ");
	if (i->mask & IN_Q_OVERFLOW)
		printf("IN_Q_OVERFLOW ");
	if (i->mask & IN_UNMOUNT)
		printf("IN_UNMOUNT ");
	printf("\n");

	if (i->len > 0)
		printf("        name = %s\n", i->name);
}
static void update_eeprom(int offset, unsigned char *data, int len)
{
	int fd, ret;
	fd = open(EEPROM_DIR, O_RDWR | O_EXCL);
	if (fd < 0)
	{
		perror("open file error");
		exit(0);
	}
	lseek(fd, offset, SEEK_SET);
	ret = write(fd, data, len);
	if (ret < len)
	{
		perror("eeprom write data error");
	}
	close(fd);
}
static void check_diff_file()
{
	int i, fd_tmp1;
	char buf;
	fd_tmp1 = open(TMP_DIR, O_RDONLY | O_NOCTTY | O_EXCL);
	if(fd_tmp1 == -1)
	{
		perror("check_diff_file open file error");
		exit(1);
	}
	for (i = 0; i < (1 << 17); i++)
	{
		read(fd_tmp1, buf, 1);
		if(buf != ptr_cmp[i])
		{
			ptr_cmp[i] =  buf;
			printf("diff char: loc:%d, buf:%X\r\n", i, buf);
			//copy to eeprom  update_eeprom(i, &buf, 1);
		}

	}
}
static void check_notify_event(struct inotify_event *i)
{
	if (i->mask & IN_MODIFY)
	{
		printf("modify file\r\n");
		check_diff_file();
	}
}

static void prepare_file()
{
	int fd, fd_tmp1, fd_tmp2, len, i;
	char *tmp;
	unsigned char buf[256];
	fd = open(EEPROM_DIR, O_RDWR | O_EXCL);
	if(fd == -1)
	{
		perror("prepare_file open EEPROM_DIR error");
		exit(1);
	}
	fd_tmp1 = open(TMP_DIR, O_RDWR | O_NOCTTY | O_CREAT | O_EXCL);
	if(fd_tmp1 == -1)
	{
		perror("prepare_file open TMP_DIR error");
		exit(1);
	}
//  fd_tmp2 = open( "/tmp/systeminfor2", O_RDWR | O_NOCTTY | O_CREAT | O_EXCL);
	ptr_cmp = (char *) malloc(1 << 17); //128KB
	if (ptr_cmp == NULL)
	{
		perror("malloc error");
		exit(0);
	}
	tmp = ptr_cmp;

	for (i = 0; i < (1 << 12); i++)
	{
		read(fd, buf, 256);
		write(fd_tmp1, buf, 256);
		memcpy(tmp, buf, 256);
		tmp += 256;
	}
	close(fd);
	close(fd_tmp1);
	//close (fd_tmp2);
}
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

int main(int argc, char *argv[])
{
	int inotifyFd, wd, j;
	char buf[BUF_LEN] __attribute__ ((aligned(8)));
	ssize_t numRead;
	char *p;
	struct inotify_event *event;
	prepare_file();
	inotifyFd = inotify_init(); /* Create inotify instance */
	if (inotifyFd == -1)
	{
		perror("inotify_init");
		exit(1);
	}

	wd = inotify_add_watch(inotifyFd, TMP_DIR, IN_MODIFY);
	if (wd == -1)
		perror("inotify_add_watch");

	for (;;)
	{ /* Read events forever */
		numRead = read(inotifyFd, buf, BUF_LEN);
		if (numRead == 0)
			printf("read() from inotify fd returned 0!");

		if (numRead == -1)
		{
			perror("read");
			//  exit (0);
		}

		printf("Read %ld bytes from inotify fd\n", (long) numRead);

		/* Process all of the events in buffer returned by read() */

		for (p = buf; p < buf + numRead;)
		{
			event = (struct inotify_event *) p;
			//displayInotifyEvent (event);
			check_notify_event(event);
			p += sizeof(struct inotify_event) + event->len;
		}
	}
	free(ptr_cmp);
	exit(EXIT_SUCCESS);
}

