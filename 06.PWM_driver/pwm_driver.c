/*
 * pwm_write.c - PWM driver đơn giản.
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h> 
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/pwm.h>

static dev_t device_number; /* Khai báo device number */
static struct class *my_class; /* Khai báo struct class */
static struct cdev my_device; /* Khai báo struct character device */

/* PWM config value */
struct pwm_device *pwm0 = NULL;
int default_duty = 2500000;/* 50% */
int period = 5000000;

static char kernel_buffer[255] = "50%\n";

#define DRIVER_NAME "pwmdevice"
#define DRIVER_CLASS "pwmclass"

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
	printk(KERN_DEBUG"%s: Read was called!\n",__func__);

	if(copy_to_user(user_buffer,kernel_buffer, strlen(kernel_buffer)))
		return -1;

	return strlen(kernel_buffer);
}

/**
 * Hàm này được gọi khi write(device_file) từ user space
 */
static ssize_t driver_write(struct file *File, const char *user_buffer, size_t count, loff_t *offset) {
	int val;

	printk(KERN_DEBUG"%s: Write was called!\n",__func__);

	/* Copy data from user_buffer and convert to int */
	if(kstrtoint_from_user(user_buffer, count, 0, &val))
		return -1;

	/* Set PWM on time */
	if(val < 0 || val > 100) /* Check duty value from 0% to 100% or not */
		printk(KERN_DEBUG"Invalid duty\n");
	else {	
		pwm_config(pwm0, period*val/100, period);
		sprintf(kernel_buffer,"%d%\n",val);
		printk(KERN_DEBUG"Set PWM duty: %s",kernel_buffer);
	}
	return count;
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

	printk(KERN_DEBUG"%s: Init PWM driver!\n",__func__);

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

	pwm0 = pwm_request(0, "pwm0");
	if(pwm0 == NULL) {
		printk(KERN_DEBUG"Failed to request pwm0\n");
		goto register_failed;
	}

	pwm_config(pwm0, default_duty , period); 
	pwm_enable(pwm0);

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
	pwm_disable(pwm0);
	pwm_free(pwm0);
 	cdev_del(&my_device);
	device_destroy(my_class, device_number);
	class_destroy(my_class);
	unregister_chrdev_region(device_number, 1);
	printk(KERN_DEBUG"%s: Exit PWM driver!\n",__func__);
}

module_init(ModuleInit);
module_exit(ModuleExit);

/* Information */
MODULE_LICENSE("GPL"); /* GNU Public License v2 or later */
MODULE_AUTHOR("Embedded-System VN");
MODULE_DESCRIPTION("PWM driver");

