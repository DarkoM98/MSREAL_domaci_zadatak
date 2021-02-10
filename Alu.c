#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#define BUFF_SIZE 20

MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;
enum formati{bin, dec, hex};
enum formati format;
int pos = 0;
int endRead = 0;
int allow_input = 1;
int rega, regb, regc, regd, result, carriage;
int result_bin[8]= {0,0,0,0,0,0,0,0};

int alu_open(struct inode *pinode, struct file *pfile);
int alu_close(struct inode *pinode, struct file *pfile);
ssize_t alu_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t alu_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = alu_open,
	.read = alu_read,
	.write = alu_write,
	.release = alu_close,
};


int alu_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened file\n");
		return 0;
}

int alu_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed file\n");
		return 0;
}

ssize_t alu_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int ret;
	char buff[BUFF_SIZE];
	long int len;
	allow_input = 1;
	if (endRead){
		endRead = 0;
		pos = 0;
		printk(KERN_INFO "Succesfully read from file\n");
		return 0;
	}
	if(format == hex) len = scnprintf(buff,BUFF_SIZE, "0x%x %d ", result, carriage);
	else if(format == dec) len = scnprintf(buff, BUFF_SIZE, "%d %d", result, carriage);
	else if(format == bin) len = scnprintf(buff, BUFF_SIZE, "%d", result_bin[7-pos]);
	ret = copy_to_user(buffer, buff, len);
	if(ret)
		return -EFAULT;
	if(format == bin){
		pos++;
		if(pos==8){
			len = scnprintf(buff, BUFF_SIZE, " %d", carriage);
			ret = copy_to_user(buffer, buff, len);
			if(ret) return -EFAULT;
			endRead = 1;

		}
	}else endRead = 1;
	return len;
}

ssize_t alu_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
	char buff[BUFF_SIZE];
	int value;
	int ret;
	char reg;
	ret = copy_from_user(buff, buffer, length);
	if(ret)
		return -EFAULT;
	buff[length-1] = '\0';

	ret = sscanf(buff,"reg%c=%x",&reg,&value);

	if(ret==2)//two parameters parsed in sscanf
	{
		if(value < 0 || value >255){
			printk(KERN_WARNING "Broj treba da bude izmedju 0x00 i 0xff\n");
			return -1;
		}else{
			switch(reg){
				case 'a':
					rega=value;
					break;
				case 'b':
					regb=value;
					break;
				case 'c':
					regc=value;
					break;
				case 'd':
					regd=value;
					break;
				default:
					printk(KERN_WARNING "Registar moze da bude a, b, c ili d\n");
					return -1;

			};
			printk(KERN_INFO "reg%c -> 0x%x",reg ,value);
		}
	}
	else
	{	char reg1, reg2, op;
		int val1, val2;
		ret = sscanf(buff, "reg%c %c reg%c", &reg1, &op, &reg2);
		if(ret == 3){
			if(allow_input == 0){
				printk(KERN_WARNING "Operacija blokirana\nPrvo isciraj rezultat\n");
				return -1;
			}
			printk(KERN_INFO "reg%c i reg%c sa operacijom %c\n", reg1, reg2, op);
			switch(reg1){
				case 'a':
					val1 = rega;
					break;
				case 'b':
					val1 = regb;
					break;
				case 'c':
					val1 = regc;
					break;
				case 'd':
					val1 = regd;
					break;
				default:
					printk(KERN_WARNING "Reg moze da bude a,b,c ili d\n");
					return -1;
			};

			switch(reg2){
				case 'a':
					val2 = rega;
					break;
				case 'b':
					val2 = regb;
					break;
				case 'c':
					val2 = regc;
					break;
				case 'd':
					val2 = regd;
					break;
				default:
					printk(KERN_WARNING "Reg moze da bude a,b,c ili d\n");
					return -1;
			};

			switch(op){
				case '+':
					result = val1+val2;
					break;
				case '-':
					result = val1-val2;
					break;
				case 'x':
					result = val1*val2;
					break;
				case '/':
					if(val2 != 0){
					result = val1/val2;
					}else{
						printk(KERN_WARNING "Deljenje sa nulom nije dozvoljeno\n");
						return -1;
					}
					break;
				default:
					printk(KERN_WARNING "Dozvoljene operacije su +, -, x i /\n");
					return -1;
			};

			carriage = 1;
			if(result > 255) result -=255;
			else if(result < 0) result +=255;
			else carriage = 0;
			int temp;
			temp = result;
			int i = 0;
			for(i=0;i<8;i++){
				result_bin[i]=temp%2;
				temp/=2;
				printk(KERN_INFO "%d", result_bin[i]);
			}
			allow_input = 0;
			//printk(KERN_INFO "rezultat je 0x%x, prenos je %d", result, carriage);
			
		}else{
			char format0,format1,format2;
			ret = sscanf(buff, "format=%c%c%c", &format0, &format1, &format2);
			if(ret == 3){
				if(format0 == 'd' && format1 == 'e' && format2 == 'c') format = dec;
				else if(format0 == 'b' && format1 == 'i' && format2 == 'n') format = bin;
				else if(format0 == 'h' && format1 == 'e' && format2 == 'x') format = hex;
				else{
					printk(KERN_WARNING "Pogresan format\nDozvoljeni formati su bin, hex i dec\n");
					return -1;
				}
			}else{
				printk(KERN_WARNING "Pogresan format komande\nTreba da bude regx=broj ili regx ? regy\n");
				return -1;
			}
		}
	}

	return length;
}

static int __init alu_init(void)
{
   int ret = 0;

	rega = 0;
	regb = 0;
	regc = 0;
	regd = 0;
	result = 0;
	carriage = 0;
	format = hex;

   ret = alloc_chrdev_region(&my_dev_id, 0, 1, "alu");
   if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "alu_class");
   if (my_class == NULL){
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, "alu");
   if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
   printk(KERN_INFO "device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret)
	{
      printk(KERN_ERR "failed to add cdev\n");
		goto fail_2;
	}
   printk(KERN_INFO "cdev added\n");
   printk(KERN_INFO "Hello world\n");

   return 0;

   fail_2:
      device_destroy(my_class, my_dev_id);
   fail_1:
      class_destroy(my_class);
   fail_0:
      unregister_chrdev_region(my_dev_id, 1);
   return -1;
}

static void __exit alu_exit(void)
{
   cdev_del(my_cdev);
   device_destroy(my_class, my_dev_id);
   class_destroy(my_class);
   unregister_chrdev_region(my_dev_id,1);
   printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(alu_init);
module_exit(alu_exit);
