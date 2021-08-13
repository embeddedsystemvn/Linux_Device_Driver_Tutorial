/*
 * helloworld.c - Kernel module example
 */
#include <linux/module.h>

/*
 * Hàm này được gọi khi module được load vào kernel
 */
static int __init ModuleInit(void) {
	printk(KERN_INFO"%s: Hello, Kernel!\n",__func__);
	return 0;
}

/*
 * Hàm này được gọi khi module bị xóa khỏi kernel
 */
static void __exit ModuleExit(void) {
	printk(KERN_INFO"%s: Goodbye, Kernel\n",__func__);
}

module_init(ModuleInit);
module_exit(ModuleExit);

/* Information */
MODULE_LICENSE("GPL"); /* GNU Public License v2 or later */
MODULE_AUTHOR("Embedded-System VN");
MODULE_DESCRIPTION("Helloworld Kernel Module");
