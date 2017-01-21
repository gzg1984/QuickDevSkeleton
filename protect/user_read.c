#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(void)
{
	int fd, ret;
	unsigned char buf[20] = {0};

	fd = open("/dev/pipe", O_RDONLY);
	if (fd < 0) {
		printf("Cannot open /dev/char0\n");
		exit(EXIT_FAILURE);
	}
	ret = read(fd, buf, 20);
	printf("Read %d bytes\n", ret);
		
	return 0;
}

