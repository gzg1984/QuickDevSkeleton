#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>

#define PIPE 'z'
#define PIPE_RESET _IO(PIPE, 1)

int main(void)
{
	int fd, ret;
	fd = open("/dev/pipe", O_RDWR);
	if (fd < 0) {
		perror("Cannot open /dev/pipe");
		exit(1);
	}
	
	ret = ioctl(fd, PIPE_RESET);
	if (ret != 0) {
		perror("pipe reset error.");
		exit(1);
	}
	printf("Pipe reset ok.\n");
	return 0;
}

