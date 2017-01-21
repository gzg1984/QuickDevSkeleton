#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>

int main(void)
{
	int fd, ret;
	unsigned char buf[20];

	/* 准备一个20字节的缓冲区，清空 */
	memset(buf, 0, 20);

	fd = open("/dev/mychar0", O_RDONLY);
	if (fd < 0) {
		printf("Cannot open /dev/mychar0\n");
		exit(EXIT_FAILURE);
	}

	/* 从缓冲区中读5字节 */
	ret = read(fd, buf, 5);
	printf("Read %d bytes： %s\n", ret, buf);
	memset(buf, 0, 20);

	/* 将读取位置向后挪5个字节 */
	if(lseek(fd, 5, SEEK_CUR) == -1)
		printf("Cannot lseek\n");
	else
		printf("Seek current ok\n");

	/* 再从缓冲区中读20字节 */
	ret = read(fd, buf, 20);
	printf("Read %d bytes: %s\n", ret, buf);
		
	return 0;
}

