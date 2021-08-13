/*
 * character_device.c - Simple Character Driver với phương thức open, close.
 */
#include <linux/module.h>
#include <linux/fs.h>

#define MYMAJOR 50

/*
 * Hàm này được gọi khi device file được open
 */
static int driver_open(struct inode *device_file, struct file *instance) {
	printk("%s: Open was called!\n",__func__);
	return 0;
}

/*
 * Hàm này được gọi khi device file được close
 */
static int driver_close(struct inode *device_file, struct file *instance) {
	printk("%s: Close was called!\n",__func__);
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close
};

/*
 * Hàm này được gọi khi module được load vào kernel
 */
static int __init ModuleInit(void) {
	int retval;
	printk("%s: Init simple character device!\n",__func__);
	/* register device numbers */
	retval = register_chrdev(MYMAJOR, "mydevice", &fops);
	if(retval == 0) {
		printk("%s: Registered Device number Major: %d, Minor: %d\n",__func__, MYMAJOR, 0);
	}
	else {
		printk("%s: Could not register device number!\n",__func__);
		return -1;
	}
	return 0;
}

/*
 * Hàm này được gọi khi module bị xóa khỏi kernel
 */
static void __exit ModuleExit(void) {
	unregister_chrdev(MYMAJOR, "mydevice");
	printk("%s: Exit simple character device!\n",__func__);
}

module_init(ModuleInit);
module_exit(ModuleExit);

/* Information */
MODULE_LICENSE("GPL"); /* GNU Public License v2 or later */
MODULE_AUTHOR("Embedded-System VN");
MODULE_DESCRIPTION("Simple Character Driver");

