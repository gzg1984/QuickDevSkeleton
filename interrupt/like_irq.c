#include <linux/module.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <asm/io.h>

#define KEY_IRQ	1
#define KEY_BUF	120

static unsigned char key_buf[KEY_BUF];
unsigned int pos;

int my_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	//print content's of key_buf
	int i;
	int ret = 0;
	
	for (i = 1; i <= pos; ++i) {
		ret += sprintf(page + ret, "%d\t",key_buf[i-1]);
		if (i % 6 == 0 && (i!=0)) {
			ret += sprintf(page + ret, "\n ");
		}
	}
	ret += sprintf(page+ret, "\n");
	pos = 0;
	return ret;
}

irqreturn_t key_service(int irq, void *data)
{
	unsigned char value = inb(0x60);
	printk("In irq %d, context is %s\n", irq, current->comm);

	if (pos < KEY_BUF) {
		key_buf[pos++] = value;
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

int __init my_init(void)
{
	int ret;
	ret = request_irq(KEY_IRQ, key_service, IRQF_SHARED, "Mykey", key_buf);
	if (ret) {
		printk("Error on request_irq\n");
		return -EIO;
	}
	pos = 0;
	create_proc_read_entry("proc_key", 0, NULL, my_proc, NULL);
	return 0;
}

void __exit my_exit(void)
{
	remove_proc_entry("proc_key", NULL);
	free_irq(KEY_IRQ,key_buf);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("1.1");

