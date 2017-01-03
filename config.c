#include <linux/module.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/pagemap.h>
#include <linux/mpage.h>
#include <linux/buffer_head.h>
#include <linux/exportfs.h>
#include <linux/mount.h>
#include <linux/vfs.h>
#include <linux/parser.h>
#include <linux/uio.h>
#include <linux/writeback.h>
#include <linux/log2.h>
#include <linux/hash.h>
#include <asm/unaligned.h>
#include "fat.h"

unsigned char msdos_name[ MSDOS_NAME ] = "BXFS_CONFAT";



int build_config_file( struct inode *dir, struct dentry *dentry, int mode )
{
	struct msdos_dir_entry de;
	struct fat_slot_info sinfo;
	struct fat_slot_info *_sinfo = &sinfo;
	struct inode *inode = NULL;
	struct super_block *sb = dir->i_sb;
	
	int cluster_num = 0;
	int err;


	//First, Create empty file

	memcpy( de.name, msdos_name, MSDOS_NAME ); //msdos_dir_entry의 name에 msdos_name배열 안에 이름을

	de.attr = 0 ? ATTR_DIR : ATTR_ARCH;
	de.lcase = 0;
	de.cdate = de.adate = 0;
	de.ctime = 0;
	de.ctime_cs = 0;
	de.time = 0;
	de.date = 0;
	de.start = cpu_to_le16(cluster_num);
	de.starthi = cpu_to_le16(cluster_num) >> 16;  //high 16bit니깐 이게 맞는거 같은데 원래 괄호안에 >> 16이 있었음
	de.size = 272 ; //length of data
	
	err = fat_add_entries( dir, &de, 1, _sinfo ); //int fat_add_entries(struct inode *dir, void *slots, int nr_slots,struct fat_slot_info *sinfo)
	inode = fat_build_inode( sb, sinfo.de, sinfo.i_pos );

	brelse( sinfo.bh );

	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		printk("[gandan] Error about inode...\n");
		goto out;
	}
	printk("[gandan] Complete Create Config File \n");

out:
	return err;
}

/*
 	Update for pre-allocation config file setting
 */
int fat_config_init(struct super_block *sb)
{
	struct fat_slot_info sinfo;
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct dentry *root_de = sb->s_root;
	struct inode *root = root_de->d_inode;
	int err;

	printk( KERN_ALERT "[cheon] fat_config_init test\n");
	
	sbi->bx_area_ratio[ BB_ETC ] = 7;
	sbi->bx_area_ratio[ BB_NORMAL ] = 70;
	sbi->bx_area_ratio[ BB_NORMAL_EVENT ] = 20;
	sbi->bx_area_ratio[ BB_PARKING ] =  7;
	sbi->bx_area_ratio[ BB_MANUAL ] = 2;
	sbi->bx_area_ratio[ BB_IMAGE ] = 2;

#if 0
//	printk( KERN_ALERT "sinfo.de->start : %d, sbi->sec_per_clus : %d, sbi->data_start : %d \n",sinfo.de->start, sbi->sec_per_clus, sbi->data_start ); 
//	printk( KERN_ALERT "sinfo.de->start : %d \n", sinfo.de->start ); 
//	printk( KERN_ALERT "sbi->sec_per_clus : %u \n", sbi->sec_per_clus); 
	//printk( KERN_ALERT "sbi->data_start : %d \n", sbi->data_start ); 
	
	if( !fat_scan( root, msdos_name, &sinfo ) ) //파일 이름을 기반으로 fat_scan
	{
		brelse( sinfo.bh );
		printk( KERN_ALERT "[cheon] config file already exist\n");
		goto exist;
	}
	
	//First - create 0 file
	printk( KERN_ALERT "[cheon] Create config file \n ");
	err = build_config_file( root, root_de, 0 ); //root inode, root dentry, mode

	
exist:
#endif	
	return 0;
}
EXPORT_SYMBOL_GPL( fat_config_init );
