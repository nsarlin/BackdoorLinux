#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xa6942b40, "module_layout" },
	{ 0x50eedeb8, "printk" },
	{ 0x7258431b, "kobject_del" },
	{ 0x1e6d26a8, "strstr" },
	{ 0x268cc6a2, "sys_close" },
	{ 0x37a0cba, "kfree" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x33d169c9, "_copy_from_user" },
	{ 0x16a864ec, "kmem_cache_alloc_trace" },
	{ 0x780303b5, "malloc_sizes" },
	{ 0x8d15b2a9, "pv_cpu_ops" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

