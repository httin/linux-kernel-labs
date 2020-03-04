/*
 * SO2 Lab - Filesystem drivers
 * Exercise #1 (no-dev filesystem)
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>

MODULE_DESCRIPTION("Simple no-dev filesystem");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

#define MYFS_BLOCKSIZE		4096
#define MYFS_BLOCKSIZE_BITS	12
#define MYFS_MAGIC		0xbeefcafe
#define LOG_LEVEL		KERN_WARNING

/* declarations of functions that are part of operation structures */

static int myfs_mknod(struct inode *dir,
		struct dentry *dentry, umode_t mode, dev_t dev);
static int myfs_create(struct inode *dir, struct dentry *dentry,
		umode_t mode, bool excl);
static int myfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);

/* TODO 2: define super_operations structure */
static const struct super_operations myfs_super_ops = {
	.statfs		= simple_statfs,		// stat -f or df
	.drop_inode	= generic_drop_inode,
};

static const struct inode_operations myfs_dir_inode_operations = {
	/* TODO 5: Fill dir inode operations structure. */
	.create         = myfs_create,	
	.lookup         = simple_lookup,
	.link           = simple_link,
	.unlink         = simple_unlink,
	.mkdir          = myfs_mkdir,
	.rmdir          = simple_rmdir,
	.mknod          = myfs_mknod,
	.rename         = simple_rename,
};

static const struct inode_operations myfs_file_inode_operations = {
	/* TODO 6: Fill file inode operations structure. */
	.getattr		= simple_getattr,

};

static const struct file_operations myfs_file_operations = {
	/* TODO 6: Fill file operations structure. */
	.llseek         = generic_file_llseek,
	.read_iter      = generic_file_read_iter,
	//...
	.write_iter     = generic_file_write_iter,
	//...
	.mmap           = generic_file_mmap,
};

static const struct address_space_operations myfs_aops = {
	/* TODO 6: Fill address space operations structure. */
	.readpage 		= simple_readpage,
	//.writepage 		= _writepage,
	.write_begin 	= simple_write_begin,
	.write_end 		= simple_write_end,
};

struct inode *myfs_get_inode(struct super_block *sb, const struct inode *dir,
		int mode)
{
	struct inode *inode = new_inode(sb);

	printk(LOG_LEVEL "%s: type=%s ",
		__FUNCTION__, S_ISDIR(mode) ? "dir" : (S_ISREG(mode)) ? "file" : "unknown");

	if (!inode)
		return NULL;
	/* TODO 3: fill inode structure
	 *     - mode, uid, gid, ino
	 *     - atime,ctime,mtime
	 */
	inode_init_owner(inode, dir, mode);
	inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);

	/* TODO 5: Init i_ino using get_next_ino */
	inode->i_ino = get_next_ino();

	/* TODO 6: Initialize address space operations. */
	inode->i_mapping->a_ops = &myfs_aops;

	if (S_ISDIR(mode)) {
		/* TODO 3: set inode operations for simple dir inodes, defined in VFS */
		/*
		 * @i_op is a pointer to 'struct inode_operations', which are, for a
		 * directory, creating a new entry, listing entries, deleting entries,etc.
		 *
		 * @i_fop is a pointer to 'struct file_operations' with the file
		 * structure associated with the inode, such as read, write, and lseek.
		 */
		inode->i_op = &simple_dir_inode_operations;
		inode->i_fop = &simple_dir_operations;

		/* TODO 5: use myfs_dir_inode_operations for inode operations (i_op) */
		inode->i_op = &myfs_dir_inode_operations;

		/* TODO 3: directory inodes start off with i_nlink == 2 (for "." entry).
		 * Directory link count should be incremented (use inc_nlink).
		 */
		inc_nlink(inode);
	}

	/* TODO 6: Set file inode and file operations for regular files
	 * (use the S_ISREG macro).
	 */
	if (S_ISREG(mode))
	{
		inode->i_op = &myfs_file_inode_operations;
		inode->i_fop = &myfs_file_operations;
	}
	printk("Inode has %d link(s)\n", inode->i_nlink);

	return inode;
}

/* TODO 5: Implement myfs_mknod, myfs_create, myfs_mkdir. */
static int myfs_mknod(struct inode *dir,
		struct dentry *dentry, umode_t mode, dev_t dev)
{
	struct inode *inode;

	printk(LOG_LEVEL "%s: ", __FUNCTION__);

	inode = myfs_get_inode(dir->i_sb, dir, mode);
	
	if (inode == NULL)
		return -ENOSPC;

	d_instantiate(dentry, inode);
	dget(dentry);
	dir->i_mtime = dir->i_ctime = current_time(inode);

	return 0;
}

static int myfs_create(struct inode *dir, struct dentry *dentry,
		umode_t mode, bool excl)
{
	printk(LOG_LEVEL "%s: ", __FUNCTION__);
	return myfs_mknod(dir, dentry, mode | S_IFREG, 0);
}

static int myfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	int ret;

	printk(LOG_LEVEL "%s: ", __FUNCTION__);
	ret = myfs_mknod(dir, dentry, mode | S_IFDIR, 0);
	if (ret != 0)
		return ret;

	inc_nlink(dir);

	return 0;
}

static int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
	/* The root inode is the inode of the file system root directory (i.e. /).
	 * Initialization is done when the file system is mounted */
	struct inode *root_inode;	
	struct dentry *root_dentry;

	printk(LOG_LEVEL "%s: ", __FUNCTION__);
	/* TODO 2: fill super_block
	 *   - blocksize, blocksize_bits (size = 2^bits)
	 *   - magic (filesystem's magic number)
	 *   - super operations
	 *   - maxbytes (max filesystem size)
	 */
	sb->s_maxbytes = MAX_LFS_FILESIZE;
	sb->s_blocksize = MYFS_BLOCKSIZE;
	sb->s_blocksize_bits = MYFS_BLOCKSIZE_BITS;
	sb->s_magic = MYFS_MAGIC;
	sb->s_op = &myfs_super_ops;
	sb->s_time_gran = 1;

	/* mode = directory & access rights (755) */
	root_inode = myfs_get_inode(sb, NULL,
			S_IFDIR | S_IRWXU | S_IRGRP |
			S_IXGRP | S_IROTH | S_IXOTH);

	printk(LOG_LEVEL "Root inode information:\n"
			"address=0x%p, i_mode=%x, i_uid=%d\n"
			"i_gid=%d, i_ino=%ld, i_rdev=%d, inode has %d link(s)\n", 
    		root_inode, root_inode->i_mode, root_inode->i_uid.val, 
    		root_inode->i_gid.val, root_inode->i_ino, 
    		root_inode->i_rdev, root_inode->i_nlink);

	if (!root_inode)
		return -ENOMEM;

	/* Allocate the root dentry */
	root_dentry = d_make_root(root_inode);
	if (!root_dentry)
		goto out_no_root;
	sb->s_root = root_dentry;
	printk(LOG_LEVEL "Dentry name %s", root_dentry->d_iname);

	return 0;

out_no_root:
	/* free an inode */
	iput(root_inode);
	return -ENOMEM;
}

static struct dentry *myfs_mount(struct file_system_type *fs_type,
		int flags, const char *dev_name, void *data)
{
	printk(LOG_LEVEL "%s: ", __FUNCTION__);
	/* TODO 1: call superblock mount function */
	/* mounts a file system that is not on a physical device */
	return mount_nodev(fs_type, flags, data, myfs_fill_super);
}

/* TODO 1: define file_system_type structure */
static struct file_system_type myfs_fs_type = {
	.owner = THIS_MODULE,
	.name = "myfs",
	.mount = myfs_mount,
	/* unmounts a filesystem that is not on a physical device, see </fs/super.c> */
	.kill_sb = kill_litter_super,
	.fs_flags = FS_USERNS_MOUNT,
};

static int __init myfs_init(void)
{
	int err;

	/* TODO 1: register a filesystem type, see /proc/filesystems */
	err = register_filesystem(&myfs_fs_type);
	if (err) {
		printk(LOG_LEVEL "%s: register_filesystem failed\n", __FUNCTION__);
		return err;
	} else {
		printk(LOG_LEVEL "%s: register_filesystem %s succeed\n", 
			__FUNCTION__, myfs_fs_type.name);
	}

	return 0;
}

static void __exit myfs_exit(void)
{
	printk(LOG_LEVEL "%s: unmount", __FUNCTION__);
	/* TODO 1: unregister */
	unregister_filesystem(&myfs_fs_type);
}

module_init(myfs_init);
module_exit(myfs_exit);
