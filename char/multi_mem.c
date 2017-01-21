#include <linux/module.h>/* 模块必须的头文件 */
#include <linux/sched.h> /* 包含了很多常用的宏和函数声明 */
#include <linux/fs.h>	 /* char驱动必备 */
#include <linux/cdev.h>	 /* 包含cdev相关内容 */
#include <asm/uaccess.h> /* 和用户态交换数据 */

#define BUF_LEN  1024	 /* 缓冲区大小 */
#define MY_MAJOR 60	 /* 缺省的主设备号 */
#define DEV_NUM 4	 /* 缓冲区数量 */

unsigned int major = MY_MAJOR;
module_param(major, uint, 0444);

unsigned int buf_lens[DEV_NUM];
unsigned int num;
module_param_array(buf_lens, uint, &num, 0444);
MODULE_PARM_DESC(buf_lens, "Choose 4 memory buffers' size, do not exceed 4KByte, default size is 1KByte");
/* 可以在模块安装时指定每个缓冲区的大小 */

struct mydev {
	char *start, *end;
	char *wp;
	unsigned int buf_size;
};
/* 设备结构体 */

static struct mydev *multi_devs[DEV_NUM];
/* 设备结构体数组的全局指针 */

struct cdev multi_cdev;
/* 由于4个缓冲区设备对应一套操作函数集，所以只需要一个cdev就可以注册了 */


static int my_open(struct inode *inode, struct file *filp)
{
/* 从inode中获得用户态打开的设备文件的从设备号 */
	int minor = iminor(inode);

/* 将和从设备号对应的结构体指针存入file对象的private_data中 */
	filp->private_data = multi_devs[minor];
	return 0;
}

static int my_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	/* 获得设备结构体指针 */
	struct mydev *dev = filp->private_data;

	/* 分析和获取有效的读取长度 */
	if ((*f_pos) >= (dev->wp - dev->start))
		return 0;
	count = min(count, (size_t)(dev->wp - dev->start - (*f_pos)));
	if (count <= 0)
		return 0;

	/* 内核空间-->用户空间 */
	if (copy_to_user(buf, (dev->start + (*f_pos)), count))
		return -EFAULT;

	/* 修改读写指针，返回实际从设备读取的字节数 */
	*f_pos += count;
	return count;
}


static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct mydev *dev = filp->private_data;

	/* 获得有效的写长度 */
	count = min(count, (size_t)(dev->end - dev->wp));
	if (0 == count)
		return 0;

	/* 用户空间-->内核空间 */
	if (copy_from_user(dev->wp, buf, count))
		return -EFAULT;

	/* 更新wp，返回实际写入缓冲区的字节数 */
	dev->wp += count;
	return count;
}

static loff_t my_llseek(struct file *filp, loff_t offset, int origin)
{
	struct mydev *dev = filp->private_data;
    loff_t res;

	printk("origin = %d; offset = %lld\n", origin, offset);

    switch (origin) {
		case 1:	/* 相对文件当前位置的偏移 */
			offset += filp->f_pos;
			break;
        case 2: /* 相对文件结尾位置的偏移 */
			offset += dev->buf_size;
			break;
	}
	/* 偏移量小于0或者超过缓冲区范围返回错误值 */
	if ((offset < 0) || (offset > dev->buf_size)) {
		res = -EINVAL;
		goto out;
        }

	filp->f_pos = offset;
	res = filp->f_pos;
out:
	return res;
}

static struct file_operations my_fops = {
	.owner 		= THIS_MODULE,
	.open  		= my_open,
	.release 	= my_release,
	.read 		= my_read,
	.write 		= my_write,
	.llseek 	= my_llseek,
};

static void release_mydev(struct mydev **devs, int num)
{
	int i;
	/* kfree(NULL)是安全的 */
	for (i=0; i<num; i++) {
		kfree(devs[i]->start);
		kfree(devs[i]);
	}
}

static int setup_mydev(struct mydev **devs, int num)
{
	int i;

	/* 根据设备的数量初始化设备结构体 */
	for(i=0; i<num; i++) {
		devs[i] = (struct mydev *)kzalloc(sizeof(struct mydev), GFP_KERNEL);
		if (NULL == devs[i]) {
			release_mydev(devs, i+1);
			return -ENOMEM;
		}

		/* 根据buf_lens[i]决定缓冲区大小，不能超过4KB，为0则采用缺省值 */
		if (buf_lens[i] >= 4096) {
			printk("Buffer size cannot exceed 4096 bytes.\n");
			devs[i]->buf_size = 4096;
		} else if (buf_lens[i] == 0) {
			devs[i]->buf_size = BUF_LEN;
		} else {
			devs[i]->buf_size = buf_lens[i];
		}

		devs[i]->start = (char *)kzalloc(devs[i]->buf_size, GFP_KERNEL);
		if (NULL == devs[i]->start) {
			release_mydev(devs, i+1);
			return -ENOMEM;
		}
		devs[i]->end = devs[i]->start + devs[i]->buf_size;
		devs[i]->wp  = devs[i]->start;
	}

	/* 所有的设备结构体和缓冲区分配成功后，返回0 */
	return 0;
}


static int __init my_init(void)
{
	int ret;
	dev_t dev_num;

	/* 分配设备结构体和缓冲区 */
	ret = setup_mydev(multi_devs, DEV_NUM);
	if (ret)
		return ret;

	/* 根据设备数量申请设备号，从(major,0)到(major,3)共4个 */
	if (major) {
		dev_num = MKDEV(major, 0);
		ret = register_chrdev_region(dev_num, DEV_NUM, "Multi schar");
	} else {
		ret = alloc_chrdev_region(&dev_num, 0, DEV_NUM, "Multi dchar");
		major = MAJOR(dev_num);
	}
	if (ret) {
		release_mydev(multi_devs, DEV_NUM);
		printk("Cannot allocate dev num\n");
		return -1;
	}

/* 用一个cdev将4个设备与一套file_operations关联，并向内核注册cdev */
	dev_num = MKDEV(major, 0);
	cdev_init(&multi_cdev, &my_fops);
	multi_cdev.owner = THIS_MODULE;
	cdev_add(&multi_cdev, dev_num, DEV_NUM);

	return 0;
}


static void __exit my_exit(void)
{
	cdev_del(&multi_cdev);
	unregister_chrdev_region(MKDEV(major, 0), DEV_NUM);
	release_mydev(multi_devs, DEV_NUM);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("nolan@uplooking.com");
MODULE_VERSION("0.1");

