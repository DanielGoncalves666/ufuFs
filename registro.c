#include<linux/fs.h>
#include<linux/version.h> /* For LINUX_VERSION_CODE & KERNEL_VERSION */
#include<linux/module.h> /* For module related macros, ... */


static struct file_system_type ufuFS =
{
	name: "ufuFS",
	mount:  ufuFS_mount,
	kill_sb: kill_block_super,
	owner: THIS_MODULE
};

static struct dentry *ufuFS_mount(struct file_system_type *fs_type, int flags, const char *devname, void *data)
{
	printk(KERN_INFO "ufuFS: devname = %s\n", devname);

	/* sfs_fill_super this will be called to fill the super block */
	return mount_bdev(fs_type, flags, devname, data, &sfs_fill_super);
}



static struct super_operations ufuFS_sops;
static struct inode_operations ufuFS_iops;
static struct file_operations ufuFS_fops;
static struct address_space_operations ufuFS_aops;




static int __init ufuFS_init(void)
{
	int err;

	err = register_filesystem(&ufuFS);
	return err;
}

static void __exit ufuFS_exit(void)
{
	unregister_filesystem(&ufuFS);
}

module_init(ufuFS_init);
module_exit(ufuFS_exit);
