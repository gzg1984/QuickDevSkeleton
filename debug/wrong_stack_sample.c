#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
struct task_struct * temp_thread;
static int test_thread(void* data)
{
	void* addr;
	void* panic_pointer;
	//void* b;
	long a=10;
	long b;
	addr=kallsyms_lookup_name("do_fork");
	panic_pointer=kallsyms_lookup_name("panic");
	printk("panic_pointer=%p\n",panic_pointer);

asm("movq %1,%%rax\n\t"
"movq %%rax,%0\n\t"
"push %%rax\n\t"
"movq $0x0,%%rdi\n\t"
"callq %%rax"
:"=r"(b)
:"r"(panic_pointer)
:"%rax"
);

/*
asm("movq %1,%%rax\n\t"
"push %%rax\n\t"
"callq %%rax"
:"=r"(b)
:"r"(panic_pointer)
:"%rax"
);
*/
	//panic("panic myself\n");
	printk("b is %p\n",b);

	return 0;

}
static int __init my_init(void)
{
	temp_thread=kthread_create(test_thread,NULL,"test_task");
	if(temp_thread)
{
wake_up_process(temp_thread);
}
	return 0;
}

static void __exit my_exit(void)
{
	/** do nothing**/
	return ;
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
