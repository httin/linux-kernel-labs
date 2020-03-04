#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>	/* task_struct{} */
#include <linux/fdtable.h>	/* files_struct[] */
#include <linux/fs_struct.h>/* fs_struct{} */
#include <linux/fs.h>		/* filp_open(), file{}, inode{}, <linux/dcache.h> */
#include <linux/fcntl.h>
#include <linux/syscalls.h>	/* sys_open */

#include <asm/uaccess.h>	/* get_fs(), set_fs() */
#include <asm/segment.h>	/* segment_t */

/* https://www.kernel.org/doc/html/latest/core-api/timekeeping.html */
#include <linux/timekeeping.h> /* ktime_get_real(), ktime_get() */

//  Define the module metadata.
#define MODULE_NAME "test"
MODULE_AUTHOR("tinht <tinht25@viettel.com.vn>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A kernel test module");
MODULE_VERSION("1.0");

#undef DEBUG

//  Define the name parameter.
static char *name = "tinht";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");

struct file *filp = NULL;

/****************** KERNEL FILES APIs ******************/
static struct file *file_open(const char *path, int flags, int mode)
{
	struct file *filp = NULL;
	mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs(); // get limit address
    set_fs(KERNEL_DS); // set limit address = KERNEL DATA SEGMENT

    /* open a file from kernelspace, see /fs/open.c */
    filp = filp_open(path, flags, mode); 
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

static void file_close(struct file *file) 
{
    if (filp_close(file, NULL) < 0)
    	printk(KERN_DEBUG "%s: filp_close: error\n", MODULE_NAME);
}

int file_read(struct file *file, unsigned char *data, size_t count, unsigned long long offset) 
{
    mm_segment_t oldfs;
    ssize_t ret = -EBADF;

    oldfs = get_fs();
    set_fs(KERNEL_DS);

    if(file) {
    	ret = vfs_read(file, data, count, &offset);
	}
    
    set_fs(oldfs);
    return ret;
}   

int file_write(struct file *file, unsigned char *data, unsigned int size, unsigned long long offset) 
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(KERNEL_DS);

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int file_sync(struct file *file) 
{
    vfs_fsync(file, 0);
    return 0;
}

#ifdef DO_NOT
void open_file(char *filename)
{
	int fd;
		
	mm_segment_t old_fs = get_fs();
	/* modify the current process address limits. In case of sys_open(), we want to
	 * tell the kernel that pointers from within the kernel address space are safe*/
	set_fs(KERNEL_DS);
		
	fd = sys_open(filename, O_RDONLY|O_CREAT, 0);
	if (fd >= 0) {
	  printk(KERN_DEBUG "%s: sys_open: OK, fd = %d\n", MODULE_NAME, fd);
	  sys_close(fd);
	}
	set_fs(old_fs);
}
#endif


static int __init test_init(void)
{
	struct files_struct *cur_files = current->files;
	struct fs_struct *cur_fs = current->fs;	
	struct inode *cur_inode;	
	struct dentry* cur_dentry;

    pr_info("%s: module loaded at 0x%p, jiffies: %ld\n",
             MODULE_NAME, test_init, jiffies);

    filp = file_open("/home/tinht25/tinht.txt", O_RDONLY|O_CREAT, 0);
    cur_inode = filp->f_inode;
    cur_dentry = filp->f_path.dentry;
    
    if (cur_inode != NULL)
    {
    	printk(KERN_DEBUG "Inode Info: address=0x%p, i_mode=%x, i_uid=%d, i_gid=%d\n"
    		"i_ino=%ld, i_nlink=%u, i_rdev=%d\n", 
    		cur_inode, cur_inode->i_mode, cur_inode->i_uid.val, cur_inode->i_gid.val, 
    		cur_inode->i_ino, cur_inode->i_nlink, cur_inode->i_rdev);
    }

    if (cur_dentry != NULL)
    {
    	printk(KERN_DEBUG "Dentry Info: file %s(%lld) is at 0x%p, d_inode at 0x%p\n",
    		cur_dentry->d_iname, filp->f_count.counter, filp, cur_dentry->d_inode);
    }

    
    if (cur_files != NULL)
        pr_emerg("files_struct: 0x%p\n", cur_files);

    if (cur_fs != NULL)
    {
        pr_emerg("fs_struct: 0x%p\n", cur_fs);
        printk(KERN_DEBUG "fs_struct: root: %s, pwd: %s\n",
        	cur_fs->root.dentry->d_iname, cur_fs->pwd.dentry->d_iname);
    }

    return 0;
}

static void __exit test_exit(void)
{
	file_close(filp);
    pr_info("%s: module unloaded from 0x%p, jiffies: %ld\n", MODULE_NAME, test_exit, jiffies);
}

module_init(test_init);
module_exit(test_exit);
