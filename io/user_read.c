#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


int main(void)
{
	int fd;
	unsigned long value;
	int ret;
	int pos = 0;
	
	fd = open("/dev/my8139", O_RDONLY);
	if (fd == -1)
		exit(1);

	for (pos = 0; pos<256; pos+=4) {
		ret = read(&value, fd, 4);
		printf("Reg 0x%x : 0x%lx\n", pos, value);
	}

	return 0;
}
