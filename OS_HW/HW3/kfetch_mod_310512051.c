#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mutex.h>
#include <linux/utsname.h>
#include <linux/cpu.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/sysinfo.h>
#include <linux/jiffies.h>
#include<asm/io.h>
#include "kfetch.h"
#define MAJOR_NUM 236
#define DEVICE_NAME "kfetch"
       
char DEFAULT_LOGO[8][25] = {  "                  ",
		              "     .-.          ",
		              "    (.. |         ",
		              "    <>  |         ",
		              "   / --- \\        ",
		              "  ( |   | |       ",
		              "|\\_)___/\\)/\\      ",
		              "<__)------(__/    "
		             };
		             
char info_temp[8][1024];
//memset(info_temp,'\0',sizeof(info_temp));
		        
/************variable**************/
static int mask_func_info = KFETCH_FULL_INFO;



struct mutex kfetch_mutex;


dev_t dev=0;
//static struct class *dev_class;
//static struct cdev *kfetch_cdev;    
             
static int __init  kfetch_init(void);
static void __exit  kfetch_exit(void);
/*****************driver function***************************/
static ssize_t kfetch_read(struct file *filp,char __user *buffer,size_t length,loff_t *offset);
static ssize_t kfetch_write(struct file *filp,const char __user *buffer,size_t length,loff_t *offset);
static int kfetch_open(struct inode *inode,struct file *file);
static int kfetch_release(struct inode *inode,struct file *file);
/***********************************************************/


/*******************driver function descriotion*********************/
static const struct file_operations kfetch_ops = {
    .owner      = THIS_MODULE,
    .read       = kfetch_read,
    .write       = kfetch_write,
    .open       = kfetch_open,
    .release    = kfetch_release
};

int   k=0;

static ssize_t kfetch_read(struct file *filp,char __user *buffer,size_t length,loff_t *offset){
	
	char *kfetch_buf = (char*)kmalloc(1024,GFP_KERNEL);
	//char *temp1 = (char*)kmalloc(1024,GFP_KERNEL);
	//char *temp2 = (char*)kmalloc(1024,GFP_KERNEL);
	char *num2s = (char*)kmalloc(20,GFP_KERNEL);
	int   len=0;
	int   i=0,j=0;
	if(mask_func_info != (0<<6)){
		k=0;
	}
	
	
	// hostname   0
	strcpy(info_temp[k],init_uts_ns.name.nodename);
	strcat(info_temp[k],"\n");
	k++;	
	// Add separator line   1
	memset(kfetch_buf,'-',strlen(init_uts_ns.name.nodename));
	strcpy(info_temp[k],kfetch_buf);
	strcat(info_temp[k],"\n");
	memset(kfetch_buf,'0',1024);
	k++;
	//KERNEL release   2
	if (mask_func_info & KFETCH_RELEASE){
           	strcpy(kfetch_buf,"Kernel:   ");
        	strcat(kfetch_buf,init_uts_ns.name.release);
        	strcat(kfetch_buf,"\n");
        	strncpy(info_temp[k],kfetch_buf,strlen(kfetch_buf)+1);
        	memset(kfetch_buf,'0',1024);
        	k++;
    	}
    	// CPU name    3
    	if (mask_func_info & KFETCH_CPU_MODEL) {
        	struct cpuinfo_x86 *c = &cpu_data(0);
        	strcpy(kfetch_buf,"CPU:      ");
        	strcat(kfetch_buf,c->x86_model_id);
        	strcat(kfetch_buf,"\n");
        	strncpy(info_temp[k], kfetch_buf,strlen(kfetch_buf)+1);
        	memset(kfetch_buf,'0',1024);
        	k++;
    	}
    	
    	// NUM CPUS   4
    	if (mask_func_info & KFETCH_NUM_CPUS) {
               	strcpy(kfetch_buf,"CPUs:     ");
        	sprintf(num2s,"%d",num_online_cpus());
        	strcat(kfetch_buf,num2s);
        	strcat(kfetch_buf," / ");
        	sprintf(num2s,"%d",num_possible_cpus());
        	strcat(kfetch_buf,num2s);
        	strcat(kfetch_buf, "\n");
        	strncpy(info_temp[k],kfetch_buf,strlen(kfetch_buf)+1);
         	memset(kfetch_buf,'0',1024);
        	memset(num2s,'0',20);
        	k++;
    	}
    	// MEM   5
    	if (mask_func_info & KFETCH_MEM) {
        	struct sysinfo mem;
        	si_meminfo(&mem);
        	strcpy(kfetch_buf,"Mem:      ");
        	sprintf(num2s,"%lu MB / %lu MB\n",mem.freeram * mem.mem_unit /1024/1024, mem.totalram * mem.mem_unit /1024/1024);
        	strcat(kfetch_buf,num2s);
        	strncpy(info_temp[k],kfetch_buf,strlen(kfetch_buf)+1);
        	memset(kfetch_buf,'0',1024);
        	memset(num2s,'0',20);
        	k++;        
    	}
    	// PROCS 6
    	if (mask_func_info & KFETCH_NUM_PROCS) {
    		struct file *file;
    		char file_buf[64];
      		char *substr;
    		char *cur;
    		int err=0;
     		
    		file = filp_open("/proc/loadavg", O_RDONLY, 0);
    		if (IS_ERR(file)) {
    			err = PTR_ERR(file);
           		printk(KERN_ERR "error opening file%d\n",err);
            		return err;
       		}
       		kernel_read(file, file_buf, sizeof(file_buf), &file->f_pos);      		
       		cur = file_buf;
        	substr = strsep(&cur," /");
        	substr = strsep(&cur," /");      	
        	substr = strsep(&cur," /"); 
        	substr = strsep(&cur," /"); 
        	substr = strsep(&cur," /"); 
        	printk("%s",substr);
        	
        	strcpy(kfetch_buf,"Procs:    ");
        	//sprintf(num2s,"%d\n",num_procs);
        	//strcat(kfetch_buf,num2s);
        	strcat(kfetch_buf,substr);
        	strcat(kfetch_buf,"\n");
        	strncpy(info_temp[k],kfetch_buf,strlen(kfetch_buf)+1);
        	memset(kfetch_buf,'0',1024);
        	memset(num2s,'0',20);
        	k++; 
        	filp_close(file, NULL);  
    	}
    	//uptime  7
    	if (mask_func_info & KFETCH_UPTIME) {
        	unsigned long uptime_msecs = jiffies_to_msecs(jiffies);
        	unsigned long uptime_mins = uptime_msecs/1000/60;

       		strcpy(kfetch_buf,"Uptime:   ");
       		sprintf(num2s,"%lu mins\n",uptime_mins);
       		strcat(kfetch_buf,num2s);
       		strncpy(info_temp[k],kfetch_buf,strlen(kfetch_buf)+1);
       		memset(kfetch_buf,'0',1024);
        	memset(num2s,'0',20);
        	k++;
    	}
    	
	memset(kfetch_buf,'\0',1024);
		
	while(i<8 || j<k){
		if(j==k){
			strcat(kfetch_buf,DEFAULT_LOGO[i]);
			strcat(kfetch_buf,"\n");
			len += strlen(kfetch_buf);
			j--;
		}
		else{
			strcat(kfetch_buf,DEFAULT_LOGO[i]);
			strcat(kfetch_buf,info_temp[j]);
			len += strlen(kfetch_buf);
		}
		i++;
		j++;
	}
		
	if(copy_to_user(buffer, kfetch_buf, 1024)){
		pr_alert("Failed to copy to user\n");
		return 0;
	}
	
	kfree(kfetch_buf);
    	return len;

}


static ssize_t kfetch_write(struct file *filp,const char __user *buffer,size_t length,loff_t *offset){
	int mask_info;
	
	if(copy_from_user(&mask_info, buffer, length)){
		pr_alert("Failed to copy from user\n");
		return 0;
	}
			
	mask_func_info = mask_info;
	return length;

}


static int kfetch_open(struct inode *inode,struct file *file){
	mutex_init(&kfetch_mutex);
	if(mutex_is_locked(&kfetch_mutex)){
		pr_alert("Failed to get mutex lock\n");
		return (-EBUSY);
	}
	mutex_lock(&kfetch_mutex);
	return 0;
}


static int kfetch_release(struct inode *inode,struct file *file){
	mutex_unlock(&kfetch_mutex);
	printk("Device File closed...!!!\n");
	return 0;
}

static int __init kfetch_init(void){
	printk("Call init\n");
	if(register_chrdev(MAJOR_NUM,DEVICE_NAME,&kfetch_ops)<0){
		printk("Can not get major %d\n",MAJOR_NUM);
		return (-EBUSY);
	}
	printk("my device is started and the major number is %d\n",MAJOR_NUM);
	return 0;
}
static void __exit  kfetch_exit(void){
	unregister_chrdev(MAJOR_NUM,DEVICE_NAME);
	printk("call exit\n");
}


module_init(kfetch_init);
module_exit(kfetch_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("310512051");
MODULE_DESCRIPTION("OS HW3");
MODULE_VERSION("22.04");




