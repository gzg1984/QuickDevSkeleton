#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>

int main(void)
{
	int fd;

	fd = open("/dev/pci0", O_RDONLY);
	if (fd < 0) {
		printf("Cannot open /dev/pci0\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}

