#include <linux/kernel.h>//strToInt
#include <linux/device.h>
#include <linux/string.h>//strlen, strchr
#include <linux/module.h>// MODULE_LICENSE
#include <linux/init.h>//module_init, module_exit
#include <linux/fs.h>//register_chrdev_region, alloc_chrdev_region
#include <linux/types.h>// dev_t
#include <linux/cdev.h>//struct cdev,cdevadd
#include <linux/kdev_t.h>//MAJOR,MINOR, MKDEV
#include <linux/uaccess.h>//copytouser,copyfromuser
#include <linux/errno.h>
#define BUFF_SIZE 20

MODULE_LICENSE("Dual BSD/GPL");
dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;
#define num_of_minors 6
//int alu[num_of_minors];
//int pos = 0;
int endRead = 0;

int rega=0;
int regb=0;
int regc=0;
int regd=0;
int res=0;
int carriage=0;

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
  int minor = MINOR(pfile->f_inode->i_rdev);
  if (endRead){
    endRead = 0;    
    printk(KERN_INFO "Succesfully read from file\n");
    return 0;
  }
  if(minor == 5){
  	len = scnprintf(buff, BUFF_SIZE, "0x%x %d\n", res, carriage);
  	ret = copy_to_user(buffer, buff, len);
  }else if(minor >= 0 && minor < 4){

	int val = 0;
	switch(minor){
		case 0:
			val = rega;
			break;
		case 1:
			val = regb;
			break;
		case 2:
			val = regc;
			break;
		case 3:
			val = regd;
			break;
	};
	len = scnprintf(buff, BUFF_SIZE, "0x%x\n", val);
	ret = copy_to_user(buffer, buff, len);
	  //printk(KERN_WARNING "Nije moguce citati iz fajla\n");
  }
  if(ret)
    return -EFAULT;
  endRead = 1;  
  return len;
}

ssize_t alu_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
  char buff[BUFF_SIZE];
  int value;
  int ret;
  int minor = MINOR(pfile->f_inode->i_rdev);
  ret = copy_from_user(buff, buffer, length);
  if(ret)
    return -EFAULT;
  buff[length-1] = '\0';
  if(minor >=0 && minor < 4){
  	ret = sscanf(buff,"%x",&value);
  	if(value < 0 || value > 255){
  		printk(KERN_WARNING "Vrednost registra mora biti izmedju 0x00 i 0xff\n");
  		return -1;
  	}
  	if(ret == 1){
  	switch(minor){
  		case 0:
  			rega = value;
  			break;
  		case 1:
  			regb = value;
  			break;
  		case 2:
  			regc = value;
  			break;
  		case 3:
  			regd = value;
  			break;
  		default:
  			printk(KERN_WARNING "los format dodele vrednosti registru\nTreba da bude: echo hex_broj > /dev/alu_regx\n");
  			
  		};
  	
  	//rega = value;
  	printk(KERN_INFO "Succesfully wrote value %d to reg%c\n", value, minor+'a');       
  	}else printk(KERN_WARNING "los format komande\n");
  }else if(minor == 4){
  		char reg1, reg2, op;
  		int val1, val2;
  		
  		printk(KERN_INFO "usao u op\n");
  		
  		ret = sscanf(buff,"reg%c %c reg%c", &reg1, &op, &reg2);
  		if(reg1 < 'a' || reg1 > 'd' || reg2 < 'a' || reg2 > 'd'){
  			printk(KERN_WARNING "Registar moze biti a,b,c ili d\n");
  			return -1;
  		}
  		
  		
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
  		}
  		
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
  		}
  		
  		switch(op){
  			case '+':
  				res = val1+val2;
  				break;
  			case '-':
  				res = val1-val2;
  				break;
  			case 'x':
  				res = val1*val2;
  				break;
  			case '/':
  				if(val2 != 0) res = val1/val2;
  				else{
  					printk(KERN_WARNING "Deljenje sa 0 nije dozvoljeno\n");
  					return 0;
  				}
  				break;
  			default:
  				printk(KERN_WARNING "Operacija moze da bude +,-,x ili /\n");
  				goto ret;
  		}

		if(res < 0){
			res+=255;
			carriage = 1;
		}else if(res > 255){
			res-=255;
			carriage = 1;
		}else carriage = 0;
  		
  		printk(KERN_INFO "reg%c %c reg%c", reg1, op, reg2);
  		
  	}
ret:
  return length;
}

static int __init alu_init(void)
{
  int ret = 0;
  int i=0;
  char buff[10];
 
  //for (i = 0; i < num_of_minors; i++){
    //  alu[i] = 0;
 // }
  ret = alloc_chrdev_region(&my_dev_id, 0, num_of_minors, "alu");
  if (ret){
    printk(KERN_INFO "failed to register char device\n");
    return ret;
  }
  printk(KERN_INFO "char device region allocated\n");

  my_class = class_create(THIS_MODULE, "alu_class");
  if (my_class == NULL){
    printk(KERN_ERR "failed to create class\n");
    goto fail_0;
  }
  printk(KERN_INFO "class created\n");

  printk(KERN_INFO "creating device\n");
/*  for (i = 0; i < num_of_minors; i++)
  {
    printk(KERN_INFO "created nod %d\n", i);
    scnprintf(buff, 10, "alu%d", i);
    my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), i), NULL, buff);
    if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
    }

 }
 */
 
  for(i = 0; i<4;i++){
//	  printk(KERN_INFO "created nod alu_rega\n");
	  sprintf(buff, "alu_reg%c", 'a'+i);
  	  printk(KERN_INFO "created nod %s\n", buff);
	  my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), i), NULL, buff);
	  if(my_device == NULL){
		  printk(KERN_INFO "failed to create device\n");
		  goto fail_1;
	  }
  }
  
  	printk(KERN_INFO "created nod alu_op\n");
	//sprintf(buff, "alu_reg%d", 'a'+i);
	my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 4), NULL, "alu_op");
	if(my_device == NULL){
	  printk(KERN_INFO "failed to create device\n");
	  goto fail_1;
	}
  
  
  	printk(KERN_INFO "created nod alu_res\n");
	//sprintf(buff, "alu_reg%d", 'a'+i);
	my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 5), NULL, "alu_res");
	if(my_device == NULL){
	  printk(KERN_INFO "failed to create device\n");
	  goto fail_1;
	}
  


  printk(KERN_INFO "device created\n");

  my_cdev = cdev_alloc();	
  my_cdev->ops = &my_fops;
  my_cdev->owner = THIS_MODULE;
  ret = cdev_add(my_cdev, my_dev_id, num_of_minors);
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
  int i = 0;

  cdev_del(my_cdev);
  for (i = 0; i < num_of_minors; i++) // every node made must be destroyed
    device_destroy(my_class, MKDEV(MAJOR(my_dev_id), i));
  class_destroy(my_class);
  
  unregister_chrdev_region(my_dev_id,num_of_minors);
  printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(alu_init);
module_exit(alu_exit);
