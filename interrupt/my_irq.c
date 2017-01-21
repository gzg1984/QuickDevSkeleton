#include <linux/module.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>

#define KEY_BUF	120

static unsigned char key_buf[KEY_BUF];
unsigned int pos;

int my_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	//print content's of key_buf 
}

irqreturn_t key_service(int irq, void *data)
{
	// value = inb(0x60)
	//fill value to key_buf
}

int __init my_init(void)
{
	//request_irq; irq = 1
	create_proc_read_entry("proc_key", 0, NULL, my_proc, NULL);
	return 0;
}

void __exit my_exit(void)
{
	remove_proc_entry("proc_key", NULL);
	//free_irq
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("1.1");

