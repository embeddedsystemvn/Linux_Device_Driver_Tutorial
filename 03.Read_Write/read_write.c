/*
 * read_write.c - Tạo device file tự động và phương thức read, write.
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h> 
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

static char kernel_buffer[255];
static int buffer_size;

static dev_t device_number; /* Khai báo device number */
static struct class *my_class; /* Khai báo struct class */
static struct cdev my_device; /* Khai báo struct character device */

#define DRIVER_NAME "mydevice"
#define DRIVER_CLASS "myclass"

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

	printk(KERN_DEBUG"%s: Read was called!\n",__func__);

	datasize = min(count, buffer_size);
	if(copy_to_user(user_buffer, kernel_buffer, datasize))
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
	
	printk(KERN_DEBUG"Data receive from user space: %s \n",kernel_buffer);
	buffer_size = datasize;

	return buffer_size ; 
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

	printk(KERN_DEBUG"%s: Init simple character device!\n",__func__);

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
	return 0;

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
 	cdev_del(&my_device);
	device_destroy(my_class, device_number);
	class_destroy(my_class);
	unregister_chrdev_region(device_number, 1);
	printk(KERN_DEBUG"%s: Exit simple character device!\n",__func__);
}

module_init(ModuleInit);
module_exit(ModuleExit);

/* Information */
MODULE_LICENSE("GPL"); /* GNU Public License v2 or later */
MODULE_AUTHOR("Embedded-System VN");
MODULE_DESCRIPTION("Auto create device file and read write function");

