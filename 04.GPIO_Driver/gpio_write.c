/*
 * gpio_write.c - GPIO driver với chức năng on/off LED và đọc trạng thái chân input.
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h> 
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

static char kernel_buffer[255];

static dev_t device_number; /* Khai báo device number */
static struct class *my_class; /* Khai báo struct class */
static struct cdev my_device; /* Khai báo struct character device */

#define DRIVER_NAME "gpiodevice"
#define DRIVER_CLASS "gpioclass"

/*
 * Hàm này được gọi khi open(device_file) từ user space
 */
static int driver_open(struct inode *device_file, struct file *instance) {
	printk(KERN_DEBUG"%s: Open was called!\n",__func__);
	return 0;
}

/*
 * Hàm này được gọi khi close(device_file) từ user space
 */
static int driver_close(struct inode *device_file, struct file *instance) {
	printk(KERN_DEBUG"%s: Close was called!\n",__func__);
	return 0;
}

/**
 * Hàm này được gọi khi read(device_file) từ user space
 */
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offset) {
	int datasize;
	char value[3] = " \n";

	printk(KERN_DEBUG"%s: Read was called!\n",__func__);

	datasize = min(count, sizeof(value));
	
	/* Read trạng thái GPIO2 */
	printk(KERN_DEBUG"Value of switch: %d\n", gpio_get_value(2));
	value[0] = gpio_get_value(2) + '0';

	if(copy_to_user(user_buffer, &value, datasize))
		return -1;

	return datasize;
}

/**
 * Hàm này được gọi khi write(device_file) từ user space
 */
static ssize_t driver_write(struct file *File, const char *user_buffer, size_t count, loff_t *offset) {
	int datasize;

	printk(KERN_DEBUG"%s: Write was called!\n",__func__);

	datasize = min(count, sizeof(kernel_buffer));

	if(copy_from_user(kernel_buffer, user_buffer, datasize))
		return -1;

	/* Set GPIO1 */
	switch(kernel_buffer[0]) {
		case '0':
			gpio_set_value(1, 0);
			printk(KERN_DEBUG"Set LED-GPIO to Low!\n");			
			break;
		case '1':
			gpio_set_value(1, 1);
			printk(KERN_DEBUG"Set LED-GPIO to High!\n");
			break;
		default:
			printk(KERN_DEBUG"Invalid Input!\n");
			break;
	}
	return datasize;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.read = driver_read,
	.write = driver_write
};

/*
 * Hàm này được gọi khi module được load vào kernel
 */
static int __init ModuleInit(void) {

	printk(KERN_DEBUG"%s: Init GPIO driver!\n",__func__);

	/* Cấp phát động device number */
	if( alloc_chrdev_region(&device_number, 0, 1, DRIVER_NAME) < 0) {
		printk(KERN_DEBUG"Failed to allocate device number\n");
		return -1;
	}
	printk(KERN_DEBUG"Registered Device number Major: %d, Minor: %d\n", MAJOR(device_number), MINOR(device_number));

	/* Tạo device class */
	if((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
		printk(KERN_DEBUG"Failed to create device class\n");
		goto class_failed;
	}

	/* Tạo device file */
	if(device_create(my_class, NULL, device_number, NULL, DRIVER_NAME) == NULL) {
		printk(KERN_DEBUG"Failed to create device file!\n");
		goto devicefile_failed;
	}

	/* Khởi tạo device file */
	cdev_init(&my_device, &fops);

	/* Đăng ký device vào kernel */
	if(cdev_add(&my_device, device_number, 1) == -1) {
		printk(KERN_DEBUG"Failed to register device to kernel\n");
		goto register_failed;
	} 

	/* gpio1 for LED*/
	if(gpio_request_one(1, GPIOF_INIT_HIGH , "LED-GPIO")){
	printk(KERN_DEBUG"Failed to config LED-GPIO\n");
		goto register_failed;
	}
	/* gpio2 for Switch*/
	if(gpio_request_one(2, GPIOF_IN , "SW-GPIO")){
	printk(KERN_DEBUG"Failed to config SW-GPIO\n");
		goto gpio_failed;
	}
	return 0;

gpio_failed:
	gpio_free(1);
register_failed:
	device_destroy(my_class, device_number);
devicefile_failed:
	class_destroy(my_class);
class_failed:
	unregister_chrdev_region(device_number, 1);
	return -1;
}

/*
 * Hàm này được gọi khi module bị xóa khỏi kernel
 */
static void __exit ModuleExit(void) {
	gpio_free(1);
	gpio_free(2);
 	cdev_del(&my_device);
	device_destroy(my_class, device_number);
	class_destroy(my_class);
	unregister_chrdev_region(device_number, 1);
	printk(KERN_DEBUG"%s: Exit GPIO driver!\n",__func__);
}

module_init(ModuleInit);
module_exit(ModuleExit);

/* Information */
MODULE_LICENSE("GPL"); /* GNU Public License v2 or later */
MODULE_AUTHOR("Embedded-System VN");
MODULE_DESCRIPTION("GPIO driver ON/OFF LED and get state of SW-GPIO");

