#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#include <linux/device.h>
#include <linux/cdev.h>

#define  BUFF_SIZE      1024
#define  MAJOR_NUMBER   260
#define  DEVICE_NAME		"rma_cpuinfo"

#define  REGISTER_NUMBER 16

struct rma_cpuinfo{
	unsigned int r[REGISTER_NUMBER];
	unsigned int id;
	unsigned int cpsr;
};

enum rma_cpumode{
	USER	= 0b10000,
	FIQ		= 0b10001,
	IRQ		= 0b10010,
	SVC		= 0b10011,
	ABORT	= 0b10111,
	UNDEF	= 0b11011,
	SYS		= 0b11111,
};

static struct cdev c_dev;
static struct class *cl;
static dev_t first;

static char *buffer  = NULL;
static int   sz_data = 0;

static struct rma_cpuinfo cpuinfo;

static inline unsigned long portCORE_ID(void){
	unsigned long val;
	__asm(" mrc p15,0,%[val],c0,c0,5\n":[val] "=r" (val)::"memory");
	return val&3;
}

static void getCPU_INFO(void){
	__asm volatile(
			"stm %[cpuinfo], {r0-pc}^		\n"
			::[cpuinfo] "r" (&cpuinfo):"memory"
			);
}

static int my_open( struct inode *inode, struct file *filp ) 
{
	printk( "[rma_cpuinfo] opened\n" );
	return 0;
}

static int my_release( struct inode *inode, struct file *filp )
{
	printk( "[rma_cpuinfo] released\n" );
	return 0;
}

static ssize_t my_write( struct file *filp, const char *buf, size_t count, loff_t *f_pos )
{
	printk( "[rma_cpuinfo] write to buffer\n");

	if (BUFF_SIZE < count)  sz_data  = BUFF_SIZE;
	sz_data  = count;

	strncpy( buffer, buf, sz_data);
	return count;
}

static ssize_t my_read( struct file *filp, char *buf, size_t count, loff_t *f_pos )
{
	char temp[100];
	int err, i;

	printk( "[rma_cpuinfo] read from buffer\n" );

	memset(buffer, 0, BUFF_SIZE);

	// 구조체에 CPUINFO 저장
	cpuinfo.id = (unsigned int)portCORE_ID();
	getCPU_INFO();
	//printk( "[getCPU_INFO] complete\n" );

	// 전송을 위해 문자열 형태로 변환
	sprintf(temp, "$%d", cpuinfo.id); 
	strcpy(buffer, temp);
	for(i=0; i<REGISTER_NUMBER; i++){
		sprintf(temp, "#%08x", cpuinfo.r[i]);
		strcat(buffer, temp);
	}
	printk(buffer);

	sz_data = strlen(buffer);

	if((err = copy_to_user(buf, buffer, sz_data)) < 0)
		return err;

	return sz_data;
}

static struct file_operations vd_fops = {
	.read = my_read,
	.write = my_write,
	.open = my_open,
	.release = my_release,
};

int __init my_init( void )
{
#if 0
	struct class *virtual_buffer_dev_class;
	struct device *dev;

	if(register_chrdev( MAJOR_NUMBER, DEVICE_NAME, &vd_fops ) < 0)
		printk(KERN_ALERT"virtual_buffer driver init failed\n");
	else
		printk(KERN_ALERT"virtual_buffer driver init succeed\n");

	virtual_buffer_dev_class = class_create(THIS_MODULE, DEVICE_NAME);
	if(IS_ERR(virtual_buffer_dev_class))
		pr_err("%s: Failed to create class(virtual_buffer-dev\n", __func__);

	dev = device_create(virtual_buffer_dev_class, NULL, MKDEV(MAJOR_NUMBER, 0), NULL, DEVICE_NAME);

	buffer = (char*) kmalloc( BUFF_SIZE, GFP_KERNEL );
	memset( buffer, 0, BUFF_SIZE);
#endif
	if(alloc_chrdev_region(&first, 0, 1, DEVICE_NAME) < 0)
		return -1;
	if((cl = class_create(THIS_MODULE, DEVICE_NAME)) == NULL){
		printk(KERN_ALERT"class create failed\n");
		unregister_chrdev_region(first, 1);
		return -1;
	}
	if(device_create(cl, NULL, first, NULL, DEVICE_NAME) == NULL){
		printk(KERN_ALERT"device create failed\n");
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	cdev_init(&c_dev, &vd_fops);
	if(cdev_add(&c_dev, first, 1) == -1){
		printk(KERN_ALERT"cdev add failed\n");
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}

	buffer = (char*) kmalloc( BUFF_SIZE, GFP_KERNEL );
	memset( buffer, 0, BUFF_SIZE);

	printk( "[rma_cpuinfo] initialized\n");

	return 0;
}

void __exit my_exit( void )
{
#if 0
	unregister_chrdev( MAJOR_NUMBER, DEVICE_NAME );
#endif
	unregister_chrdev_region(first, 1);

	cdev_del(&c_dev);
	device_destroy(cl, first);
	class_destroy(cl);

	kfree( buffer );
	printk( "[rma_cpuinfo] exited\n");
}

module_init( my_init );
module_exit( my_exit );

MODULE_LICENSE( "GPL" );
